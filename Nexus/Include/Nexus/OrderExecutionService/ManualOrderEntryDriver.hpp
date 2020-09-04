#ifndef NEXUS_MANUALORDERENTRYDRIVER_HPP
#define NEXUS_MANUALORDERENTRYDRIVER_HPP
#include <vector>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class ManualOrderEntryDriver
      \brief Allows a position to be manually assigned to an account.
      \tparam OrderExecutionDriverType The type of OrderExecutionDriver to send
              non-manual Order entries to.
      \tparam AdministrationClientType The type of AdministrationClient used to
              authorize the use of a manual order entry.
   */
  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  class ManualOrderEntryDriver : private boost::noncopyable {
    public:

      //! The type of OrderExecutionDriver to send non-manual Order entries to.
      using OrderExecutionDriver =
        Beam::GetTryDereferenceType<OrderExecutionDriverType>;

      //! The type of AdministrationClient used to authorize the use of a
      //! manual order entry.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! Constructs a ManualOrderEntryDriver.
      /*!
        \param destinationName The name of the destination used to trigger a
               manual Order.
        \param orderExecutionDriver The OrderExecutionDriver to send non-manual
               Order entries to.
        \param administrationClient Initializes the AdministrationClient.
      */
      template<typename OrderExecutionDriverForward,
        typename AdministrationClientForward>
      ManualOrderEntryDriver(const std::string& destinationName,
        OrderExecutionDriverForward&& orderExecutionDriver,
        AdministrationClientForward&& administrationClient);

      ~ManualOrderEntryDriver();

      const Order& Recover(const SequencedAccountOrderRecord& orderRecord);

      const Order& Submit(const OrderInfo& orderInfo);

      void Cancel(const OrderExecutionSession& session, OrderId orderId);

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport);

      void Close();

    private:
      std::string m_destinationName;
      Beam::GetOptionalLocalPtr<OrderExecutionDriverType>
        m_orderExecutionDriver;
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::SynchronizedVector<std::unique_ptr<Order>> m_orders;
      Beam::SynchronizedUnorderedSet<OrderId> m_orderIds;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  template<typename OrderExecutionDriverForward,
    typename AdministrationClientForward>
  ManualOrderEntryDriver<OrderExecutionDriverType, AdministrationClientType>::
      ManualOrderEntryDriver(const std::string& destinationName,
      OrderExecutionDriverForward&& orderExecutionDriver,
      AdministrationClientForward&& administrationClient)
      : m_destinationName(destinationName),
        m_orderExecutionDriver(std::forward<OrderExecutionDriverForward>(
          orderExecutionDriver)),
        m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)) {
    m_openState.SetOpen();
  }

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  ManualOrderEntryDriver<OrderExecutionDriverType, AdministrationClientType>::
      ~ManualOrderEntryDriver() {
    Close();
  }

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  const Order& ManualOrderEntryDriver<OrderExecutionDriverType,
      AdministrationClientType>::Recover(
      const SequencedAccountOrderRecord& orderRecord) {
    return m_orderExecutionDriver->Recover(orderRecord);
  }

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  const Order& ManualOrderEntryDriver<OrderExecutionDriverType,
      AdministrationClientType>::Submit(const OrderInfo& orderInfo) {
    if(orderInfo.m_fields.m_destination != m_destinationName) {
      auto& order = m_orderExecutionDriver->Submit(orderInfo);
      return order;
    }
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      orderInfo.m_submissionAccount);
    if(!isAdministrator) {
      auto order = BuildRejectedOrder(orderInfo,
        "Insufficient permissions to execute a manual order.");
      auto result = order.get();
      m_orderIds.Insert(order->GetInfo().m_orderId);
      m_orders.PushBack(std::move(order));
      return *result;
    }
    auto order = std::make_unique<PrimitiveOrder>(orderInfo);
    order->With(
      [&] (OrderStatus status, const std::vector<ExecutionReport>& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          OrderStatus::NEW, orderInfo.m_timestamp);
        order->Update(updatedReport);
      });
    order->With(
      [&] (OrderStatus status, const std::vector<ExecutionReport>& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
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

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  void ManualOrderEntryDriver<OrderExecutionDriverType,
      AdministrationClientType>::Cancel(const OrderExecutionSession& session,
      OrderId orderId) {
    if(m_orderIds.Contains(orderId)) {
      return;
    }
    return m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  void ManualOrderEntryDriver<OrderExecutionDriverType,
      AdministrationClientType>::Update(const OrderExecutionSession& session,
      OrderId orderId, const ExecutionReport& executionReport) {
    if(m_orderIds.Contains(orderId)) {
      return;
    }
    return m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  void ManualOrderEntryDriver<OrderExecutionDriverType,
      AdministrationClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename OrderExecutionDriverType, typename AdministrationClientType>
  void ManualOrderEntryDriver<OrderExecutionDriverType,
      AdministrationClientType>::Shutdown() {
    m_openState.SetClosed();
  }
}
}

#endif
