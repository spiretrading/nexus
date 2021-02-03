#ifndef NEXUS_MANUAL_ORDER_ENTRY_DRIVER_HPP
#define NEXUS_MANUAL_ORDER_ENTRY_DRIVER_HPP
#include <vector>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Allows a position to be manually assigned to an account.
   * @param <D> The type of OrderExecutionDriver to send non-manual Order
   *        entries to.
   * @param <A> The type of AdministrationClient used to authorize the use of a
   *        manual order entry.
   */
  template<typename D, typename A>
  class ManualOrderEntryDriver {
    public:

      /**
       * The type of OrderExecutionDriver to send non-manual Order entries to.
       */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<D>;

      /**
       * The type of AdministrationClient used to authorize the use of a manual
       * order entry.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /**
       * Constructs a ManualOrderEntryDriver.
       * @param destinationName The name of the destination used to trigger a
       *        manual Order.
       * @param orderExecutionDriver The OrderExecutionDriver to send non-manual
       *        Order entries to.
       * @param administrationClient Initializes the AdministrationClient.
       */
      template<typename DF, typename AF>
      ManualOrderEntryDriver(std::string destinationName,
        DF&& orderExecutionDriver, AF&& administrationClient);

      ~ManualOrderEntryDriver();

      const Order& Recover(const SequencedAccountOrderRecord& orderRecord);

      const Order& Submit(const OrderInfo& orderInfo);

      void Cancel(const OrderExecutionSession& session, OrderId orderId);

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport);

      void Close();

    private:
      std::string m_destinationName;
      Beam::GetOptionalLocalPtr<D> m_orderExecutionDriver;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::SynchronizedVector<std::unique_ptr<Order>> m_orders;
      Beam::SynchronizedUnorderedSet<OrderId> m_orderIds;
      Beam::IO::OpenState m_openState;

      ManualOrderEntryDriver(const ManualOrderEntryDriver&) = delete;
      ManualOrderEntryDriver& operator =(
        const ManualOrderEntryDriver&) = delete;
  };

  template<typename D, typename A>
  template<typename DF, typename AF>
  ManualOrderEntryDriver<D, A>::ManualOrderEntryDriver(
    std::string destinationName, DF&& orderExecutionDriver,
    AF&& administrationClient)
    : m_destinationName(std::move(destinationName)),
      m_orderExecutionDriver(std::forward<DF>(orderExecutionDriver)),
      m_administrationClient(std::forward<AF>(administrationClient)) {}

  template<typename D, typename A>
  ManualOrderEntryDriver<D, A>::~ManualOrderEntryDriver() {
    Close();
  }

  template<typename D, typename A>
  const Order& ManualOrderEntryDriver<D, A>::Recover(
      const SequencedAccountOrderRecord& orderRecord) {
    if((*orderRecord)->m_info.m_fields.m_destination == m_destinationName) {
      auto order = std::make_unique<PrimitiveOrder>(**orderRecord);
      auto result = order.get();
      m_orderIds.Insert(order->GetInfo().m_orderId);
      m_orders.PushBack(std::move(order));
      return *result;
    } else {
      return m_orderExecutionDriver->Recover(orderRecord);
    }
  }

  template<typename D, typename A>
  const Order& ManualOrderEntryDriver<D, A>::Submit(
      const OrderInfo& orderInfo) {
    if(orderInfo.m_fields.m_destination != m_destinationName) {
      auto& order = m_orderExecutionDriver->Submit(orderInfo);
      return order;
    }
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      orderInfo.m_submissionAccount);
    if(!isAdministrator) {
      auto order = MakeRejectedOrder(orderInfo,
        "Insufficient permissions to execute a manual order.");
      auto result = order.get();
      m_orderIds.Insert(order->GetInfo().m_orderId);
      m_orders.PushBack(std::move(order));
      return *result;
    }
    auto order = std::make_unique<PrimitiveOrder>(orderInfo);
    order->With([&] (auto status, const auto& reports) {
      auto& lastReport = reports.back();
      auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
        OrderStatus::NEW, orderInfo.m_timestamp);
      order->Update(updatedReport);
    });
    order->With([&] (auto status, const auto& reports) {
      auto& lastReport = reports.back();
      auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
        OrderStatus::FILLED, orderInfo.m_timestamp);
      updatedReport.m_lastQuantity = order->GetInfo().m_fields.m_quantity;
      updatedReport.m_lastPrice = order->GetInfo().m_fields.m_price;
      updatedReport.m_lastMarket = m_destinationName;
      order->Update(updatedReport);
    });
    auto result = order.get();
    m_orderIds.Insert(order->GetInfo().m_orderId);
    m_orders.PushBack(std::move(order));
    return *result;
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    if(m_orderIds.Contains(orderId)) {
      return;
    }
    return m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    if(m_orderIds.Contains(orderId)) {
      return;
    }
    return m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename D, typename A>
  void ManualOrderEntryDriver<D, A>::Close() {
    m_openState.Close();
  }
}

#endif
