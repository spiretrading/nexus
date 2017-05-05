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
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
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
      using SequencedMarketDataValue =
        boost::variant<SequencedBboQuote, SequencedTimeAndSale>;
      struct SecurityEntry {
        Security m_security;
        boost::optional<std::deque<SequencedBboQuote>> m_bboQuotes;
        Beam::Queries::Sequence m_lastBboSequence;
        boost::optional<std::deque<SequencedTimeAndSale>> m_timeAndSales;
        Beam::Queries::Sequence m_lastTimeAndSaleSequence;
      };
      friend class BacktesterMarketDataClient;
      friend class BacktesterServiceClients;
      mutable Beam::Threading::Mutex m_mutex;
      mutable Beam::Threading::ConditionVariable m_marketDataCondition;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      std::unordered_map<Security, SecurityEntry> m_securityEntries;
      TestEnvironment m_testEnvironment;
      Beam::Routines::RoutineHandler m_eventLoopRoutine;
      Beam::IO::OpenState m_openState;

      static const boost::posix_time::ptime& GetTimestamp(
        const SequencedMarketDataValue& value);
      boost::optional<SequencedMarketDataValue> LoadNextValue(
        SecurityEntry& entry);
      void UpdateMarketData(const Security& security,
        const SequencedMarketDataValue& value);
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

  inline const boost::posix_time::ptime& BacktesterEnvironment::GetTimestamp(
      const SequencedMarketDataValue& value) {
    if(auto quote = boost::get<const SequencedBboQuote>(&value)) {
      return Beam::Queries::GetTimestamp(quote->GetValue());
    } else {
      return Beam::Queries::GetTimestamp(
        boost::get<const SequencedTimeAndSale&>(value).GetValue());
    }
  }

  inline boost::optional<BacktesterEnvironment::SequencedMarketDataValue>
      BacktesterEnvironment::LoadNextValue(SecurityEntry& entry) {
    boost::optional<SequencedMarketDataValue> nextValue;
    if(entry.m_bboQuotes.is_initialized()) {
      if(entry.m_bboQuotes->empty()) {
        if(entry.m_lastBboSequence != Beam::Queries::Sequence::Last()) {
          auto startPoint =
            [&] () -> Beam::Queries::Range::Point {
              if(entry.m_lastBboSequence == Beam::Queries::Sequence::First()) {
                return m_startTime;
              } else {
                return Beam::Queries::Increment(entry.m_lastBboSequence);
              }
            }();
          auto endPoint =
            [&] () -> Beam::Queries::Range::Point {
              if(m_endTime == boost::posix_time::pos_infin) {
                return Beam::Queries::Sequence::Present();
              }
              return m_endTime;
            }();
          MarketDataService::SecurityMarketDataQuery updateQuery;
          updateQuery.SetIndex(entry.m_security);
          updateQuery.SetRange(startPoint, endPoint);
          updateQuery.SetSnapshotLimit(
            Beam::Queries::SnapshotLimit::Type::HEAD, 1000);
          auto queue = std::make_shared<Beam::Queue<SequencedBboQuote>>();
          m_marketDataClient->QueryBboQuotes(updateQuery, queue);
          Beam::FlushQueue(queue, std::back_inserter(*entry.m_bboQuotes));
          if(!entry.m_bboQuotes->empty()) {
            entry.m_lastBboSequence = entry.m_bboQuotes->back().GetSequence();
          } else {
            entry.m_lastBboSequence = Beam::Queries::Sequence::Last();
          }
        }
      }
      if(!entry.m_bboQuotes->empty()) {
        nextValue = entry.m_bboQuotes->front();
      }
    }
    if(entry.m_timeAndSales.is_initialized()) {
      if(entry.m_timeAndSales->empty()) {
      }
      if(!entry.m_timeAndSales->empty() && (!nextValue.is_initialized() ||
          Beam::Queries::GetTimestamp(
          entry.m_timeAndSales->front().GetValue()) <
          GetTimestamp(*nextValue))) {
        nextValue = entry.m_timeAndSales->front();
      }
    }
    return nextValue;
  }

  inline void BacktesterEnvironment::UpdateMarketData(const Security& security,
      const SequencedMarketDataValue& value) {
    if(auto quote = boost::get<const SequencedBboQuote>(&value)) {
      m_securityEntries[security].m_bboQuotes->pop_front();
      m_testEnvironment.Update(security, *quote);
    } else {
      m_securityEntries[security].m_timeAndSales->pop_front();
    }
  }

  inline void BacktesterEnvironment::EventLoop() {
    while(true) {
      {

        // TODO: Don't hold lock when updating testEnvironment.
        boost::unique_lock<Beam::Threading::Mutex> lock{m_mutex};
        while(m_openState.IsOpen() && m_securityEntries.empty()) {
          m_marketDataCondition.wait(lock);
        }
        if(!m_openState.IsOpen()) {
          return;
        }
        Security security;
        boost::optional<SequencedMarketDataValue> nextValue;
        for(auto& securityEntry : m_securityEntries) {
          auto value = LoadNextValue(securityEntry.second);
          if(value.is_initialized()) {
            if(!nextValue.is_initialized() ||
                GetTimestamp(*value) < GetTimestamp(*nextValue)) {
              security = securityEntry.first;
              nextValue = std::move(value);
            }
          }
        }
        if(nextValue.is_initialized()) {
          UpdateMarketData(security, *nextValue);
        } else {
          m_securityEntries.clear();
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
      auto entryIterator = m_securityEntries.find(query.GetIndex());
      if(entryIterator != m_securityEntries.end()) {
        auto& entry = entryIterator->second;
        if(entry.m_bboQuotes.is_initialized()) {
          return;
        }
      }
      auto& entry = m_securityEntries[query.GetIndex()];
      entry.m_security = query.GetIndex();
      entry.m_bboQuotes.emplace();
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
