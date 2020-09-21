#ifndef NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#define NEXUS_SECURITY_ORDER_SIMULATOR_HPP
#include <unordered_set>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Handles simulating Orders submitted for a specific Security.
   * @param <C> The type of TimeClient used for Order timestamps.
   */
  template<typename C>
  class SecurityOrderSimulator {
    public:

      /** The type of TimeClient to use. */
      using TimeClient = C;

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
      std::unordered_set<std::shared_ptr<PrimitiveOrder>> m_orders;
      std::shared_ptr<Beam::StateQueue<BboQuote>> m_bboQuoteQueue;
      Beam::RoutineTaskQueue m_tasks;

      SecurityOrderSimulator(const SecurityOrderSimulator&) = delete;
      SecurityOrderSimulator& operator =(
        const SecurityOrderSimulator&) = delete;
      OrderStatus FillOrder(PrimitiveOrder& order, Money price);
      OrderStatus UpdateOrder(PrimitiveOrder& order);
      void OnBbo(const BboQuote& bboQuote);
  };

  template<typename C>
  template<typename MarketDataClient>
  SecurityOrderSimulator<C>::SecurityOrderSimulator(
      MarketDataClient& marketDataClient, const Security& security,
      Beam::Ref<TimeClient> timeClient)
      : m_timeClient(timeClient.Get()),
        m_bboQuoteQueue(std::make_shared<Beam::StateQueue<BboQuote>>()) {
    auto bboQuery = Beam::Queries::BuildCurrentQuery(security);
    marketDataClient.QueryBboQuotes(bboQuery, m_bboQuoteQueue);
    marketDataClient.QueryBboQuotes(bboQuery, m_tasks.GetSlot<BboQuote>(
      std::bind(&SecurityOrderSimulator::OnBbo, this, std::placeholders::_1)));
  }

  template<typename C>
  void SecurityOrderSimulator<C>::Submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push([=] {
      m_orders.insert(order);
      order->With([&] (auto status, auto& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          OrderStatus::NEW, order->GetInfo().m_timestamp);
        order->Update(updatedReport);
      });
      UpdateOrder(*order);
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
        auto pendingCancelReport = ExecutionReport::BuildUpdatedReport(
          reports.back(), OrderStatus::PENDING_CANCEL, m_timeClient->GetTime());
        order->Update(pendingCancelReport);
        auto cancelReport = ExecutionReport::BuildUpdatedReport(reports.back(),
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

  template<typename C>
  void SecurityOrderSimulator<C>::Recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push([=] {
      m_orders.insert(order);
      UpdateOrder(*order);
    });
  }

  template<typename C>
  OrderStatus SecurityOrderSimulator<C>::FillOrder(
      PrimitiveOrder& order, Money price) {
    order.With([&] (auto status, auto& reports) {
      auto& lastReport = reports.back();
      auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
        OrderStatus::FILLED, m_timeClient->GetTime());
      updatedReport.m_lastQuantity = order.GetInfo().m_fields.m_quantity;
      updatedReport.m_lastPrice = price;
      order.Update(updatedReport);
    });
    return OrderStatus::FILLED;
  }

  template<typename C>
  OrderStatus SecurityOrderSimulator<C>::UpdateOrder(
      PrimitiveOrder& order) {
    auto finalStatus = OrderStatus();
    auto bboQuote = m_bboQuoteQueue->Peek();
    order.With([&] (auto status, auto& reports) {
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
        auto price = [&] {
          if(side == Side::BID) {
            return bboQuote.m_ask.m_price;
          } else {
            return bboQuote.m_bid.m_price;
          }
        }();
        finalStatus = FillOrder(order, price);
      } else {
        if(side == Side::BID && bboQuote.m_ask.m_price <=
            order.GetInfo().m_fields.m_price) {
          finalStatus = this->FillOrder(order, bboQuote.m_ask.m_price);
        } else if(side == Side::ASK && bboQuote.m_bid.m_price >=
            order.GetInfo().m_fields.m_price) {
          finalStatus = this->FillOrder(order, bboQuote.m_bid.m_price);
        }
      }
    });
    return finalStatus;
  }

  template<typename C>
  void SecurityOrderSimulator<C>::OnBbo(const BboQuote& bboQuote) {
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
}

#endif
