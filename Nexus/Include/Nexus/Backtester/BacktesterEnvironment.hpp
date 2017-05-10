#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include <deque>
#include <unordered_map>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"

namespace Nexus {
namespace Details {
  struct TimerExpiredEvent {
    Beam::Threading::Mutex m_mutex;
    BacktesterTimer* m_timer;
    Beam::Threading::Timer::Result m_result;
    boost::posix_time::ptime m_timestamp;
    bool m_isTriggered;
    Beam::Threading::ConditionVariable m_expiredCondition;

    TimerExpiredEvent(BacktesterTimer& timer,
        Beam::Threading::Timer::Result result,
        boost::posix_time::ptime timestamp)
        : m_timer{&timer},
          m_result{result},
          m_timestamp{timestamp},
          m_isTriggered{false} {}
  };

  void TriggerTimer(TimerExpiredEvent& event);
}

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
        boost::optional<BboQuote> m_currentBbo;
        boost::optional<std::deque<SequencedTimeAndSale>> m_timeAndSales;
        Beam::Queries::Sequence m_lastTimeAndSaleSequence;
        std::vector<const OrderExecutionService::Order*> m_pendingOrders;
      };
      friend class BacktesterMarketDataClient;
      friend class BacktesterOrderExecutionClient;
      friend class BacktesterServiceClients;
      friend class BacktesterTimer;
      mutable Beam::Threading::Mutex m_mutex;
      mutable Beam::Threading::ConditionVariable m_eventAvailableCondition;
      MarketDataService::VirtualMarketDataClient* m_marketDataClient;
      boost::posix_time::ptime m_startTime;
      boost::posix_time::ptime m_endTime;
      std::unordered_map<Security, SecurityEntry> m_securityEntries;
      std::deque<Details::TimerExpiredEvent*> m_timerEvents;
      TestEnvironment m_testEnvironment;
      std::shared_ptr<Beam::Queue<const OrderExecutionService::Order*>>
        m_orderSubmissionQueue;
      std::unordered_map<OrderExecutionService::OrderId,
        const OrderExecutionService::Order*> m_orders;
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
      void Submit(const OrderExecutionService::Order& order);
      void Cancel(const OrderExecutionService::Order& order);
      void Expire(Details::TimerExpiredEvent& event);
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
        m_endTime{std::move(endTime)},
        m_orderSubmissionQueue{std::make_shared<
          Beam::Queue<const OrderExecutionService::Order*>>()} {}

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
      m_testEnvironment.MonitorOrderSubmissions(m_orderSubmissionQueue);
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
      auto& securityEntry = m_securityEntries[security];
      securityEntry.m_bboQuotes->pop_front();
      m_testEnvironment.Update(security, *quote);
      securityEntry.m_currentBbo = *quote;
      auto orderIterator = securityEntry.m_pendingOrders.begin();
      while(orderIterator != securityEntry.m_pendingOrders.end()) {
        auto& order = *orderIterator;
        if(order->GetInfo().m_fields.m_side == Side::BID &&
            securityEntry.m_currentBbo->m_ask.m_price <=
            order->GetInfo().m_fields.m_price) {
          m_testEnvironment.FillOrder(*order,
            securityEntry.m_currentBbo->m_ask.m_price,
            order->GetInfo().m_fields.m_quantity);
          orderIterator = securityEntry.m_pendingOrders.erase(orderIterator);
        } else if(order->GetInfo().m_fields.m_side == Side::ASK &&
            securityEntry.m_currentBbo->m_bid.m_price >=
            order->GetInfo().m_fields.m_price) {
          m_testEnvironment.FillOrder(*order,
            securityEntry.m_currentBbo->m_bid.m_price,
            order->GetInfo().m_fields.m_quantity);
          orderIterator = securityEntry.m_pendingOrders.erase(orderIterator);
        } else {
          ++orderIterator;
        }
      }
    } else {
      m_securityEntries[security].m_timeAndSales->pop_front();
    }
  }

  inline void BacktesterEnvironment::EventLoop() {
    struct MarketDataEvent {
      Security m_security;
      SequencedMarketDataValue m_value;
    };
    using Event = boost::variant<MarketDataEvent, Details::TimerExpiredEvent*>;
    while(true) {
      {
        boost::unique_lock<Beam::Threading::Mutex> lock{m_mutex};
        while(m_openState.IsOpen() && m_securityEntries.empty() &&
            m_timerEvents.empty()) {
          m_eventAvailableCondition.wait(lock);
        }
        if(!m_openState.IsOpen()) {
          return;
        }
        boost::posix_time::ptime nextTimestamp = boost::posix_time::pos_infin;
        boost::optional<Event> nextEvent;
        if(!m_timerEvents.empty() &&
            m_timerEvents.front()->m_timestamp < nextTimestamp) {
          nextTimestamp = m_timerEvents.front()->m_timestamp;
          nextEvent = m_timerEvents.front();
        }
        for(auto& securityEntry : m_securityEntries) {
          auto value = LoadNextValue(securityEntry.second);
          if(value.is_initialized() && GetTimestamp(*value) < nextTimestamp) {
            nextTimestamp = GetTimestamp(*value);
            nextEvent = MarketDataEvent{securityEntry.first,
              std::move(*value)};
          }
        }
        if(nextEvent.is_initialized()) {
          if(auto timerEvent =
              boost::get<Details::TimerExpiredEvent*>(&*nextEvent)) {
            m_timerEvents.pop_front();
            Details::TriggerTimer(**timerEvent);
          } else if(auto marketDataEvent =
              boost::get<MarketDataEvent>(&*nextEvent)) {
            UpdateMarketData(marketDataEvent->m_security,
              marketDataEvent->m_value);
          }
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
      m_eventAvailableCondition.notify_one();
    }
    Beam::Routines::FlushPendingRoutines();
  }

  inline void BacktesterEnvironment::Submit(
      const OrderExecutionService::Order& order) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto serverOrder =
      [&] {
        auto orderIterator = m_orders.find(order.GetInfo().m_orderId);
        if(orderIterator == m_orders.end()) {
          while(true) {
            auto orderSubmission = m_orderSubmissionQueue->Top();
            m_orderSubmissionQueue->Pop();
            m_orders.insert(std::make_pair(
              orderSubmission->GetInfo().m_orderId, orderSubmission));
            if(orderSubmission->GetInfo().m_orderId ==
                order.GetInfo().m_orderId) {
              return orderSubmission;
            }
          }
        }
        return orderIterator->second;
      }();
    if(serverOrder->GetInfo().m_fields.m_timeInForce.GetType() !=
        TimeInForce::Type::DAY &&
        serverOrder->GetInfo().m_fields.m_timeInForce.GetType() !=
        TimeInForce::Type::GTC) {
      m_testEnvironment.RejectOrder(*serverOrder);
      return;
    }
    if(serverOrder->GetInfo().m_fields.m_type == OrderType::LIMIT) {
      m_testEnvironment.AcceptOrder(*serverOrder);
      auto& securityEntry =
        m_securityEntries[serverOrder->GetInfo().m_fields.m_security];
      if(securityEntry.m_currentBbo.is_initialized()) {
        if(serverOrder->GetInfo().m_fields.m_side == Side::BID &&
            securityEntry.m_currentBbo->m_ask.m_price <=
            serverOrder->GetInfo().m_fields.m_price) {
          m_testEnvironment.FillOrder(*serverOrder,
            securityEntry.m_currentBbo->m_ask.m_price,
            serverOrder->GetInfo().m_fields.m_quantity);
        } else if(serverOrder->GetInfo().m_fields.m_side == Side::ASK &&
            securityEntry.m_currentBbo->m_bid.m_price >=
            serverOrder->GetInfo().m_fields.m_price) {
          m_testEnvironment.FillOrder(*serverOrder,
            securityEntry.m_currentBbo->m_bid.m_price,
            serverOrder->GetInfo().m_fields.m_quantity);
        } else {
          securityEntry.m_pendingOrders.push_back(serverOrder);
        }
      } else {
        securityEntry.m_pendingOrders.push_back(serverOrder);
      }
    } else if(serverOrder->GetInfo().m_fields.m_type == OrderType::MARKET) {
      m_testEnvironment.AcceptOrder(*serverOrder);
      auto& securityEntry =
        m_securityEntries[serverOrder->GetInfo().m_fields.m_security];
      if(securityEntry.m_currentBbo.is_initialized()) {
        if(serverOrder->GetInfo().m_fields.m_side == Side::BID) {
          m_testEnvironment.FillOrder(*serverOrder,
            securityEntry.m_currentBbo->m_ask.m_price,
            serverOrder->GetInfo().m_fields.m_quantity);
        } else if(serverOrder->GetInfo().m_fields.m_side == Side::ASK) {
          m_testEnvironment.FillOrder(*serverOrder,
            securityEntry.m_currentBbo->m_bid.m_price,
            serverOrder->GetInfo().m_fields.m_quantity);
        }
      } else {
        securityEntry.m_pendingOrders.push_back(serverOrder);
      }
    } else {
      m_testEnvironment.RejectOrder(*serverOrder);
    }
  }

  inline void BacktesterEnvironment::Cancel(
      const OrderExecutionService::Order& order) {
    const OrderExecutionService::Order* serverOrder;
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      auto orderIterator = m_orders.find(order.GetInfo().m_orderId);
      if(orderIterator == m_orders.end()) {
        return;
      }
      serverOrder = orderIterator->second;
      auto& securityEntry =
        m_securityEntries[serverOrder->GetInfo().m_fields.m_security];
      auto pendingOrderIterator =
        std::find(securityEntry.m_pendingOrders.begin(),
        securityEntry.m_pendingOrders.end(), serverOrder);
      if(pendingOrderIterator == securityEntry.m_pendingOrders.end()) {
        return;
      }
      securityEntry.m_pendingOrders.erase(pendingOrderIterator);
    }
    auto queue = std::make_shared<
      Beam::Queue<OrderExecutionService::ExecutionReport>>();
    serverOrder->GetPublisher().Monitor(queue);
    while(true) {
      auto executionReport = queue->Top();
      queue->Pop();
      if(IsTerminal(executionReport.m_status)) {
        return;
      } else if(executionReport.m_status == OrderStatus::PENDING_CANCEL) {
        m_testEnvironment.CancelOrder(*serverOrder);
        return;
      }
    }
  }

  inline void BacktesterEnvironment::Expire(
      Details::TimerExpiredEvent& event) {
    {
      boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
      m_timerEvents.push_back(&event);
    }
    m_eventAvailableCondition.notify_one();
    boost::unique_lock<Beam::Threading::Mutex> lock{event.m_mutex};
    while(!event.m_isTriggered) {
      event.m_expiredCondition.wait(lock);
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
