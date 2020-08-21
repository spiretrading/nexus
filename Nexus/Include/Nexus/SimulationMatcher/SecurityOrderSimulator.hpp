#ifndef NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#define NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#include <unordered_set>
#include <boost/noncopyable.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/SimulationMatcher/SimulationMatcher.hpp"

namespace Nexus::OrderExecutionService {

  /** Handles simulating Orders submitted for a specific Security.
      \tparam TimeClientType The type of TimeClient used for Order timestamps.
   */
  template<typename TimeClientType>
  class SecurityOrderSimulator : private boost::noncopyable {
    public:

      //! The type of TimeClient to use.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs a SecurityOrderSimulator.
      /*!
        \param marketDataClient The MarketDataClient to query.
        \param security The Security to simulate Order executions for.
        \param timeClient The TimeClient used for Order timestamps.
      */
      template<typename MarketDataClient>
      SecurityOrderSimulator(MarketDataClient& marketDataClient,
        const Security& security, Beam::Ref<TimeClient> timeClient);

      //! Submits an Order for simulated Order entry.
      /*!
        \param order The Order to submit.
      */
      void Submit(const std::shared_ptr<PrimitiveOrder>& order);

      //! Cancels a previously submitted order.
      /*!
        \param order The Order to cancel.
      */
      void Cancel(const std::shared_ptr<PrimitiveOrder>& order);

      //! Updates an Order.
      /*!
        \param order The Order to update.
        \param executionReport The ExecutionReport containing the update.
      */
      void Update(const std::shared_ptr<PrimitiveOrder>& order,
        const ExecutionReport& executionReport);

      //! Recovers a previously submitted Order.
      /*!
        \param order The Order to recover.
      */
      void Recover(const std::shared_ptr<PrimitiveOrder>& order);

    private:
      TimeClient* m_timeClient;
      boost::gregorian::date m_date;
      boost::posix_time::ptime m_marketCloseTime;
      bool m_isMocPending;
      std::unordered_set<std::shared_ptr<PrimitiveOrder>> m_orders;
      BboQuote m_bboQuote;
      Beam::RoutineTaskQueue m_tasks;

      void SetSessionTimestamps(boost::posix_time::ptime timestamp);
      OrderStatus Fill(PrimitiveOrder& order, Money price);
      OrderStatus UpdateOrder(PrimitiveOrder& order);
      void OnBbo(const BboQuote& bboQuote);
      void OnTimeAndSale(const TimeAndSale& timeAndSale);
  };

  template<typename TimeClientType>
  template<typename MarketDataClient>
  SecurityOrderSimulator<TimeClientType>::SecurityOrderSimulator(
      MarketDataClient& marketDataClient, const Security& security,
      Beam::Ref<TimeClient> timeClient)
      : m_timeClient(timeClient.Get()) {
    SetSessionTimestamps(m_timeClient->GetTime());
    auto snapshot = std::make_shared<Beam::Queue<BboQuote>>();
    marketDataClient.QueryBboQuotes(Beam::Queries::BuildLatestQuery(security),
      snapshot);
    try {
      m_bboQuote = snapshot->Pop();
    } catch(const std::exception&) {
      return;
    }
    auto query = Beam::Queries::BuildCurrentQuery(security);
    marketDataClient.QueryBboQuotes(query, m_tasks.GetSlot<BboQuote>(
      std::bind(&SecurityOrderSimulator::OnBbo, this, std::placeholders::_1)));
    marketDataClient.QueryTimeAndSales(query, m_tasks.GetSlot<TimeAndSale>(
      std::bind(&SecurityOrderSimulator::OnTimeAndSale, this,
      std::placeholders::_1)));
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push(
      [=] {
        auto isLive = true;
        order->With(
          [&] (auto status, auto& reports) {
            auto& lastReport = reports.back();
            auto nextStatus = [&] {
              if(m_bboQuote.m_bid.m_price == Money::ZERO ||
                  m_bboQuote.m_ask.m_price == Money::ZERO) {
                isLive = false;
                return OrderStatus::REJECTED;
              }
              return OrderStatus::NEW;
            }();
            auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
              nextStatus, order->GetInfo().m_timestamp);
            order->Update(updatedReport);
          });
        if(isLive) {
          m_orders.insert(order);
          UpdateOrder(*order);
        }
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Cancel(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push(
      [=] {
        order->With(
          [&] (auto status, auto& reports) {
            if(IsTerminal(status) || reports.empty()) {
              return;
            }
            auto pendingCancelReport = ExecutionReport::BuildUpdatedReport(
              reports.back(), OrderStatus::PENDING_CANCEL,
              m_timeClient->GetTime());
            order->Update(pendingCancelReport);
            auto cancelReport = ExecutionReport::BuildUpdatedReport(
              reports.back(), OrderStatus::CANCELED, m_timeClient->GetTime());
            order->Update(cancelReport);
          });
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Update(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& executionReport) {
    m_tasks.Push(
      [=] {
        order->With(
          [&] (auto status, auto& executionReports) {
            if(IsTerminal(status)) {
              return;
            }
            auto updatedReport = executionReport;
            updatedReport.m_sequence = executionReports.back().m_sequence + 1;
            if(updatedReport.m_timestamp.is_special()) {
              updatedReport.m_timestamp = m_timeClient->GetTime();
            }
            order->Update(updatedReport);
          });
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push(
      [=] {
        m_orders.insert(order);
        UpdateOrder(*order);
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::SetSessionTimestamps(
      boost::posix_time::ptime timestamp) {
    m_date = timestamp.date();
    auto easternTimestamp = Beam::TimeService::AdjustDateTime(timestamp, "UTC",
      "Eastern_Time", GetDefaultTimeZoneDatabase());
    m_marketCloseTime = Beam::TimeService::AdjustDateTime(
      boost::posix_time::ptime{easternTimestamp.date(),
      boost::posix_time::hours(16)}, "Eastern_Time", "UTC",
      GetDefaultTimeZoneDatabase());
    m_isMocPending = timestamp < m_marketCloseTime;
  }

  template<typename TimeClientType>
  OrderStatus SecurityOrderSimulator<TimeClientType>::Fill(
      PrimitiveOrder& order, Money price) {
    order.With(
      [&] (auto status, auto& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          OrderStatus::FILLED, m_timeClient->GetTime());
        updatedReport.m_lastQuantity = order.GetInfo().m_fields.m_quantity;
        updatedReport.m_lastPrice = price;
        order.Update(updatedReport);
      });
    return OrderStatus::FILLED;
  }

  template<typename TimeClientType>
  OrderStatus SecurityOrderSimulator<TimeClientType>::UpdateOrder(
      PrimitiveOrder& order) {
    OrderStatus finalStatus;
    order.With(
      [&] (auto status, auto& reports) {
        auto side = order.GetInfo().m_fields.m_side;
        finalStatus = status;
        if(status == OrderStatus::PENDING_NEW || IsTerminal(status)) {
          return;
        }
        if(order.GetInfo().m_fields.m_timeInForce.GetType() ==
            TimeInForce::Type::MOC) {
          return;
        }
        if(order.GetInfo().m_fields.m_type == OrderType::MARKET) {
          auto price = Pick(side, m_bboQuote.m_bid.m_price,
            m_bboQuote.m_ask.m_price);
          finalStatus = this->Fill(order, price);
        } else {
          if(side == Side::BID && m_bboQuote.m_ask.m_price <=
              order.GetInfo().m_fields.m_price) {
            finalStatus = this->Fill(order, m_bboQuote.m_ask.m_price);
          } else if(side == Side::ASK && m_bboQuote.m_bid.m_price >=
              order.GetInfo().m_fields.m_price) {
            finalStatus = this->Fill(order, m_bboQuote.m_bid.m_price);
          }
        }
      });
    return finalStatus;
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::OnBbo(
      const BboQuote& bboQuote) {
    if(bboQuote.m_timestamp.date() != m_date) {
      SetSessionTimestamps(bboQuote.m_timestamp);
    }
    m_bboQuote = bboQuote;
    auto i = m_orders.begin();
    while(i != m_orders.end()) {
      auto status = UpdateOrder(**i);
      if(IsTerminal(status)) {
        i = m_orders.erase(i);
      } else {
        ++i;
      }
    }
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::OnTimeAndSale(
      const TimeAndSale& timeAndSale) {
    if(timeAndSale.m_timestamp.date() != m_date) {
      SetSessionTimestamps(timeAndSale.m_timestamp);
    }
    if(m_isMocPending && timeAndSale.m_timestamp >= m_marketCloseTime &&
        timeAndSale.m_marketCenter == "TSE") {
      m_isMocPending = false;
      auto closingPrice = timeAndSale.m_price;
      auto i = m_orders.begin();
      while(i != m_orders.end()) {
        auto& order = **i;
        if(order.GetInfo().m_fields.m_timeInForce.GetType() ==
            TimeInForce::Type::MOC) {
          Fill(order, closingPrice);
          i = m_orders.erase(i);
        } else {
          ++i;
        }
      }
    }
  }
}

#endif
