#ifndef NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#define NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#include <vector>
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

  /**
   * Handles simulating Orders submitted for a specific Security.
   * @param <C> The type of TimeClient used for Order timestamps.
   */
  template<typename C>
  class SecurityOrderSimulator {
    public:

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a SecurityOrderSimulator.
       * @param marketDataClient The MarketDataClient to query.
       * @param security The Security to simulate Order executions for.
       * @param timeClient The TimeClient used for Order timestamps.
       */
      template<typename MarketDataClient>
      SecurityOrderSimulator(MarketDataClient& marketDataClient,
        const Security& security, Beam::Ref<TimeClient> timeClient);

      /**
       * Submits an Order for simulated Order entry.
       * @param order The Order to submit.
       */
      void Submit(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Cancels a previously submitted order.
       * @param order The Order to cancel.
       */
      void Cancel(const std::shared_ptr<PrimitiveOrder>& order);

      /**
       * Updates an Order.
       * @param order The Order to update.
       * @param executionReport The ExecutionReport containing the update.
       */
      void Update(const std::shared_ptr<PrimitiveOrder>& order,
        const ExecutionReport& executionReport);

      /**
       * Recovers a previously submitted Order.
       * @param order The Order to recover.
       */
      void Recover(const std::shared_ptr<PrimitiveOrder>& order);

    private:
      TimeClient* m_timeClient;
      boost::gregorian::date m_date;
      boost::posix_time::ptime m_marketCloseTime;
      bool m_isMocPending;
      std::vector<std::shared_ptr<PrimitiveOrder>> m_orders;
      BboQuote m_bboQuote;
      Beam::RoutineTaskQueue m_tasks;

      SecurityOrderSimulator(const SecurityOrderSimulator&) = delete;
      SecurityOrderSimulator& operator =(
        const SecurityOrderSimulator&) = delete;
      void SetSessionTimestamps(boost::posix_time::ptime timestamp);
      OrderStatus Fill(PrimitiveOrder& order, Money price);
      OrderStatus UpdateOrder(PrimitiveOrder& order);
      void OnBbo(const BboQuote& bboQuote);
      void OnTimeAndSale(const TimeAndSale& timeAndSale);
  };

  template<typename C>
  template<typename MarketDataClient>
  SecurityOrderSimulator<C>::SecurityOrderSimulator(
      MarketDataClient& marketDataClient, const Security& security,
      Beam::Ref<TimeClient> timeClient)
      : m_timeClient(timeClient.Get()) {
    SetSessionTimestamps(m_timeClient->GetTime());
    auto snapshot = std::make_shared<Beam::Queue<BboQuote>>();
    marketDataClient.QueryBboQuotes(Beam::Queries::MakeLatestQuery(security),
      snapshot);
    try {
      m_bboQuote = snapshot->Pop();
    } catch(const std::exception&) {
      return;
    }
    auto query = Beam::Queries::MakeCurrentQuery(security);
    marketDataClient.QueryBboQuotes(query, m_tasks.GetSlot<BboQuote>(
      std::bind(&SecurityOrderSimulator::OnBbo, this, std::placeholders::_1)));
    marketDataClient.QueryTimeAndSales(query, m_tasks.GetSlot<TimeAndSale>(
      std::bind(&SecurityOrderSimulator::OnTimeAndSale, this,
        std::placeholders::_1)));
  }

  template<typename C>
  void SecurityOrderSimulator<C>::Submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push([=] {
      auto isLive = true;
      order->With([&] (auto status, auto& reports) {
        auto& lastReport = reports.back();
        auto nextStatus = [&] {
          if(m_bboQuote.m_bid.m_price == Money::ZERO ||
              m_bboQuote.m_ask.m_price == Money::ZERO) {
            isLive = false;
            return OrderStatus::REJECTED;
          }
          return OrderStatus::NEW;
        }();
        auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
          nextStatus, order->GetInfo().m_timestamp);
        order->Update(updatedReport);
      });
      if(isLive) {
        m_orders.push_back(order);
        UpdateOrder(*order);
      }
    });
  }

  template<typename C>
  void SecurityOrderSimulator<C>::Cancel(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push([=] {
      order->With([&] (auto status, auto& reports) {
        if(IsTerminal(status) || reports.empty()) {
          return;
        }
        auto pendingCancelReport = ExecutionReport::MakeUpdatedReport(
          reports.back(), OrderStatus::PENDING_CANCEL, m_timeClient->GetTime());
        order->Update(pendingCancelReport);
        auto cancelReport = ExecutionReport::MakeUpdatedReport(reports.back(),
          OrderStatus::CANCELED, m_timeClient->GetTime());
        order->Update(cancelReport);
      });
    });
  }

  template<typename C>
  void SecurityOrderSimulator<C>::Update(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& executionReport) {
    m_tasks.Push([=] {
      order->With([&] (auto status, auto& executionReports) {
        if(IsTerminal(status) || executionReports.empty() &&
            executionReport.m_status != OrderStatus::PENDING_NEW) {
          return;
        }
        auto updatedReport = executionReport;
        if(executionReports.empty()) {
          updatedReport.m_sequence = 0;
        } else {
          updatedReport.m_sequence = executionReports.back().m_sequence + 1;
        }
        if(updatedReport.m_timestamp.is_special()) {
          updatedReport.m_timestamp = m_timeClient->GetTime();
        }
        order->Update(updatedReport);
      });
    });
  }

  template<typename C>
  void SecurityOrderSimulator<C>::Recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push([=] {
      m_orders.push_back(order);
      UpdateOrder(*order);
    });
  }

  template<typename C>
  void SecurityOrderSimulator<C>::SetSessionTimestamps(
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

  template<typename C>
  OrderStatus SecurityOrderSimulator<C>::Fill(
      PrimitiveOrder& order, Money price) {
    order.With([&] (auto status, auto& reports) {
      auto& lastReport = reports.back();
      auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
        OrderStatus::FILLED, m_timeClient->GetTime());
      updatedReport.m_lastQuantity = order.GetInfo().m_fields.m_quantity;
      updatedReport.m_lastPrice = price;
      order.Update(updatedReport);
    });
    return OrderStatus::FILLED;
  }

  template<typename C>
  OrderStatus SecurityOrderSimulator<C>::UpdateOrder(PrimitiveOrder& order) {
    return order.With([&] (auto status, auto& reports) {
      auto side = order.GetInfo().m_fields.m_side;
      if(status == OrderStatus::PENDING_NEW || IsTerminal(status) ||
          order.GetInfo().m_fields.m_timeInForce.GetType() ==
          TimeInForce::Type::MOC) {
        return status;
      }
      if(order.GetInfo().m_fields.m_type == OrderType::MARKET) {
        auto price = Pick(side, m_bboQuote.m_bid.m_price,
          m_bboQuote.m_ask.m_price);
        return Fill(order, price);
      } else if(side == Side::BID && m_bboQuote.m_ask.m_price <=
          order.GetInfo().m_fields.m_price) {
        return Fill(order, m_bboQuote.m_ask.m_price);
      } else if(side == Side::ASK && m_bboQuote.m_bid.m_price >=
          order.GetInfo().m_fields.m_price) {
        return Fill(order, m_bboQuote.m_bid.m_price);
      }
      return status;
    });
  }

  template<typename C>
  void SecurityOrderSimulator<C>::OnBbo(const BboQuote& bboQuote) {
    if(bboQuote.m_timestamp.date() != m_date) {
      SetSessionTimestamps(bboQuote.m_timestamp);
    }
    m_bboQuote = bboQuote;
    m_orders.erase(std::remove_if(m_orders.begin(), m_orders.end(),
      [&] (auto& order) {
        return IsTerminal(UpdateOrder(*order));
      }), m_orders.end());
  }

  template<typename C>
  void SecurityOrderSimulator<C>::OnTimeAndSale(
      const TimeAndSale& timeAndSale) {
    if(timeAndSale.m_timestamp.date() != m_date) {
      SetSessionTimestamps(timeAndSale.m_timestamp);
    }
    if(m_isMocPending && timeAndSale.m_timestamp >= m_marketCloseTime &&
        timeAndSale.m_marketCenter == "TSE") {
      m_isMocPending = false;
      auto closingPrice = timeAndSale.m_price;
      m_orders.erase(std::remove_if(m_orders.begin(), m_orders.end(),
        [&] (auto& order) {
          if(order->GetInfo().m_fields.m_timeInForce.GetType() ==
              TimeInForce::Type::MOC) {
            Fill(*order, closingPrice);
            return true;
          } else {
            return false;
          }
        }), m_orders.end());
    }
  }
}

#endif
