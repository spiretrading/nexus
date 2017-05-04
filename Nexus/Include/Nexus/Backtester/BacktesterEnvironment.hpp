#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include <deque>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class BacktesterEnvironment
      \brief Provides all of the services needed to run historical data.
   */
  class BacktesterEnvironment : private boost::noncopyable {
    public:

      //! Constructs a BacktesterEnvironment.
      /*!
        \param marketDataClient The MarketDataClient used to retrieve
               historical market data.
        \param startTime The initial time to retrieve data for.
      */
      BacktesterEnvironment(Beam::RefType<
        MarketDataService::VirtualMarketDataClient> marketDataClient,
        boost::posix_time::ptime startTime);

      //! Constructs a BacktesterEnvironment.
      /*!
        \param marketDataClient The MarketDataClient used to retrieve
               historical market data.
        \param startTime The initial time to retrieve data for.
        \param endTime The time to stop retrieving data.
      */
      BacktesterEnvironment(Beam::RefType<
        MarketDataService::VirtualMarketDataClient> marketDataClient,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime);

      ~BacktesterEnvironment();

      void Open();

      void Close();

    private:
      friend class BacktesterMarketDataClient;
      friend class BacktesterServiceClients;
      mutable Beam::Threading::Mutex m_mutex;
      mutable Beam::Threading::ConditionVariable m_marketDataCondition;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      std::unordered_map<Security, std::deque<SequencedBboQuote>> m_bboQuotes;
      TestEnvironment m_testEnvironment;
      Beam::Routines::RoutineHandler m_eventLoopRoutine;
      Beam::IO::OpenState m_openState;

      void EventLoop();
      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);
      void Shutdown();
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
      Beam::RefType<MarketDataService::VirtualMarketDataClient>
      marketDataClient, boost::posix_time::ptime startTime)
      : BacktesterEnvironment{Beam::Ref(marketDataClient),
          std::move(startTime), boost::posix_time::pos_infin} {}

  inline BacktesterEnvironment::BacktesterEnvironment(
      Beam::RefType<MarketDataService::VirtualMarketDataClient>
      marketDataClient, boost::posix_time::ptime startTime,
      boost::posix_time::ptime endTime)
      : m_marketDataClient{marketDataClient.Get()},
        m_startTime{std::move(startTime)},
        m_endTime{std::move(endTime)} {}

  inline BacktesterEnvironment::~BacktesterEnvironment() {
    Close();
  }

  inline void BacktesterEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataClient->Open();
      m_testEnvironment.SetTime(m_startTime);
      m_testEnvironment.Open();
      m_eventLoopRoutine = Beam::Routines::Spawn(
        std::bind(&BacktesterEnvironment::EventLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterEnvironment::EventLoop() {
    while(true) {
      {
        boost::unique_lock<Beam::Threading::Mutex> lock{m_mutex};
        while(m_openState.IsOpen() && m_bboQuotes.empty()) {
          m_marketDataCondition.wait(lock);
        }
        if(!m_openState.IsOpen()) {
          return;
        }
        auto i = m_bboQuotes.begin();
        while(i != m_bboQuotes.end()) {
          auto& entry = *i;
          if(entry.second.size() <= 1) {
            MarketDataService::SecurityMarketDataQuery historicalQuery;
            historicalQuery.SetIndex(entry.first);
            auto endRange =
              [&] {
                if(m_endTime == boost::posix_time::pos_infin) {
                  return Beam::Queries::Range::Point{
                    Beam::Queries::Sequence::Present()};
                } else {
                  return Beam::Queries::Range::Point{m_endTime};
                }
              }();
            if(entry.second.empty()) {
              if(m_testEnvironment.GetTimeEnvironment().GetTime() <
                  m_endTime) {
                historicalQuery.SetRange(
                  m_testEnvironment.GetTimeEnvironment().GetTime(), endRange);
              } else {
                historicalQuery.SetRange(Beam::Queries::Range::Empty());
              }
            } else {
              historicalQuery.SetRange(
                Beam::Queries::Increment(entry.second.front().GetSequence()),
                endRange);
            }
            if(historicalQuery.GetRange() != Beam::Queries::Range::Empty()) {
              historicalQuery.SetSnapshotLimit(
                Beam::Queries::SnapshotLimit::Type::HEAD, 1000);
              auto queue = std::make_shared<Beam::Queue<SequencedBboQuote>>();
              m_marketDataClient->QueryBboQuotes(historicalQuery, queue);
              Beam::FlushQueue(queue, std::back_inserter(entry.second));
            }
          }
          if(!entry.second.empty()) {
            m_testEnvironment.Update(entry.first,
              std::move(entry.second.front()));
            entry.second.pop_front();
          }
          if(entry.second.empty()) {
            i = m_bboQuotes.erase(i);
          } else {
            ++i;
          }
        }
      }
    }
  }

  inline void BacktesterEnvironment::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    if(query.GetRange().GetEnd() != Beam::Queries::Sequence::Last()) {
      return;
    }
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      if(m_bboQuotes.find(query.GetIndex()) != m_bboQuotes.end()) {
        return;
      }
      m_bboQuotes[query.GetIndex()] = std::deque<SequencedBboQuote>();
      m_marketDataCondition.notify_one();
    }
    Beam::Routines::FlushPendingRoutines();
  }

  inline void BacktesterEnvironment::Shutdown() {
    m_testEnvironment.Close();
    m_marketDataCondition.notify_one();
    m_eventLoopRoutine.Wait();
    m_openState.SetClosed();
  }
}

#endif
