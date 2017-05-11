#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include <deque>
#include <unordered_set>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/LockRelease.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEvent.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {

  /*! \class BacktesterEnvironment
      \brief Provides all of the services needed to run historical data.
   */
  class BacktesterEnvironment : private boost::noncopyable {
    public:

      //! Constructs a BacktesterEnvironment.
      /*!
        \param startTime The starting point of the backtester.
      */
      BacktesterEnvironment(boost::posix_time::ptime startTime);

      //! Constructs a BacktesterEnvironment.
      /*!
        \param startTime The starting point of the backtester.
        \param endTime The time to stop backtesting.
      */
      BacktesterEnvironment(boost::posix_time::ptime startTime,
        boost::posix_time::ptime endTime);

      ~BacktesterEnvironment();

      void Add(std::shared_ptr<BacktesterEvent> event);

      void Add(std::vector<std::shared_ptr<BacktesterEvent>> events);

      const TestEnvironment& GetTestEnvironment() const;

      TestEnvironment& GetTestEnvironment();

      void QueryBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      void Open();

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      TestEnvironment m_testEnvironment;
      std::unordered_set<Security> m_bboQueries;
      std::deque<std::shared_ptr<BacktesterEvent>> m_events;
      Beam::Threading::ConditionVariable m_eventAvailableCondition;
      Beam::Routines::RoutineHandler m_eventLoopRoutine;
      Beam::IO::OpenState m_openState;

      void EventLoop();
      void Shutdown();
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
      boost::posix_time::ptime startTime)
      : BacktesterEnvironment{std::move(startTime),
          boost::posix_time::pos_infin} {}

  inline BacktesterEnvironment::BacktesterEnvironment(
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime)
      : m_startTime{std::move(startTime)},
        m_endTime{std::move(endTime)} {}

  inline BacktesterEnvironment::~BacktesterEnvironment() {
    Close();
  }

  inline void BacktesterEnvironment::Add(
      std::shared_ptr<BacktesterEvent> event) {
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      auto insertIterator = std::lower_bound(m_events.begin(), m_events.end(),
        event,
        [] (auto& lhs, auto& rhs) {
          return lhs->GetTimestamp() < rhs->GetTimestamp();
        });
      m_events.insert(insertIterator, std::move(event));
    }
    m_eventAvailableCondition.notify_one();
  }

  inline void BacktesterEnvironment::Add(
      std::vector<std::shared_ptr<BacktesterEvent>> events) {
    if(events.empty()) {
      return;
    }
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      auto insertIterator = std::lower_bound(m_events.begin(), m_events.end(),
        events.front(),
        [] (auto& lhs, auto& rhs) {
          return lhs->GetTimestamp() < rhs->GetTimestamp();
        });
      m_events.insert(insertIterator, m_events.begin(), m_events.end());
    }
    m_eventAvailableCondition.notify_one();
  }

  inline const TestEnvironment& BacktesterEnvironment::
      GetTestEnvironment() const {
    return m_testEnvironment;
  }

  inline TestEnvironment& BacktesterEnvironment::GetTestEnvironment() {
    return m_testEnvironment;
  }

  inline void BacktesterEnvironment::QueryBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    if(m_bboQueries.find(query.GetIndex()) != m_bboQueries.end()) {
      return;
    }
    m_bboQueries.insert(query.GetIndex());
    auto queue = std::make_shared<Beam::Queue<SequencedBboQuote>>();
    MarketDataService::SecurityMarketDataQuery realTimeQuery;
    realTimeQuery.SetIndex(query.GetIndex());
    realTimeQuery.SetRange(m_testEnvironment.GetTimeEnvironment().GetTime(),
      Beam::Queries::Sequence::Present());
    realTimeQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::HEAD,
      1000);
    m_marketDataClient->QueryBboQuotes(query, queue);
    std::vector<SequencedBboQuote> bboQuotes;
    Beam::FlushQueue(queue, std::back_inserter(bboQuotes));
    if(bboQuotes.empty()) {
      return;
    }
    for(auto& bboQuote :
        boost::make_iterator_range(bboQuotes.begin(), bboQuotes.end() - 1)) {
      auto marketDataEvent = std::make_shared<BboQuoteBacktesterEvent>(
        query.GetIndex(), bboQuote);
    }
  }

  inline void BacktesterEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
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
      std::shared_ptr<BacktesterEvent> event;
      {
        boost::unique_lock<Beam::Threading::Mutex> lock{m_mutex};
        while(m_openState.IsOpen() && m_events.empty()) {
          m_eventAvailableCondition.wait(lock);
        }
        if(!m_openState.IsOpen()) {
          return;
        }
        {
          auto release = Beam::Threading::Release(lock);
          Beam::Routines::FlushPendingRoutines();
        }
        event = m_events.front();
        m_events.pop_front();
        m_testEnvironment.SetTime(event->GetTimestamp());
      }
      event->Execute();
    }
  }

  inline void BacktesterEnvironment::Shutdown() {
    m_eventAvailableCondition.notify_one();
    m_eventLoopRoutine.Wait();
    m_testEnvironment.Close();
    m_openState.SetClosed();
  }
}

#endif
