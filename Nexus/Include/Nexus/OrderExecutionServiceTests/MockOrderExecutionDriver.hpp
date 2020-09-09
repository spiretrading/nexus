#ifndef NEXUS_MOCKORDEREXECUTIONDRIVER_HPP
#define NEXUS_MOCKORDEREXECUTIONDRIVER_HPP
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  /*! \class MockOrderExecutionDriver
      \brief An OrderExecutionDriver used for testing purposes.
   */
  class MockOrderExecutionDriver : private boost::noncopyable {
    public:

      //! Constructs a MockOrderExecutionDriver.
      MockOrderExecutionDriver();

      //! Sets the state of any submitted Order to NEW upon submission.
      void SetOrderStatusNewOnSubmission(bool value);

      //! Finds an order with a specified client id.
      /*!
        \param orderId The Order's id.
        \return The Order with the specified <i>orderId</i>.
      */
      PrimitiveOrder& FindOrder(OrderId orderId);

      //! Adds an ExecutionReport to be used to recover an Order.
      /*!
        \param executionReport The ExecutionReport to add for recovery.
      */
      void AddRecovery(const ExecutionReport& executionReport);

      //! Returns the Publisher storing Order submissions.
      const Beam::Publisher<PrimitiveOrder*>& GetPublisher() const;

      const Order& Recover(const SequencedAccountOrderRecord& orderRecord);

      const Order& Submit(const OrderInfo& orderInfo);

      void Cancel(const OrderExecutionSession& session, OrderId orderId);

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport);

      void Close();

    private:
      bool m_setOrderStatusNewOnSubmission;
      std::unordered_map<OrderId, std::unique_ptr<PrimitiveOrder>> m_orders;
      Beam::SequencePublisher<PrimitiveOrder*> m_publisher;
      std::unordered_map<OrderId, std::vector<ExecutionReport>> m_recoveries;
  };

  inline MockOrderExecutionDriver::MockOrderExecutionDriver()
    : m_setOrderStatusNewOnSubmission(false) {}

  inline void MockOrderExecutionDriver::SetOrderStatusNewOnSubmission(
      bool value) {
    m_setOrderStatusNewOnSubmission = value;
  }

  inline PrimitiveOrder& MockOrderExecutionDriver::FindOrder(OrderId orderId) {
    return *m_orders.at(orderId);
  }

  inline void MockOrderExecutionDriver::AddRecovery(
      const ExecutionReport& executionReport) {
    m_recoveries[executionReport.m_id].push_back(executionReport);
  }

  inline const Beam::Publisher<PrimitiveOrder*>& MockOrderExecutionDriver::
      GetPublisher() const {
    return m_publisher;
  }

  inline const Order& MockOrderExecutionDriver::Recover(
      const SequencedAccountOrderRecord& orderRecord) {
    auto orderIterator = m_orders.find((*orderRecord)->m_info.m_orderId);
    if(orderIterator != m_orders.end()) {
      return *orderIterator->second;
    }
    auto fullReports = (*orderRecord)->m_executionReports;
    auto recoveryIterator = m_recoveries.find((*orderRecord)->m_info.m_orderId);
    if(recoveryIterator != m_recoveries.end()) {
      fullReports.insert(fullReports.end(), recoveryIterator->second.begin(),
        recoveryIterator->second.end());
    }
    auto& order = *m_orders.insert(std::make_pair(
      (*orderRecord)->m_info.m_orderId, std::make_unique<PrimitiveOrder>(
      OrderRecord{(*orderRecord)->m_info,
      std::move(fullReports)}))).first->second;
    return order;
  }

  inline const Order& MockOrderExecutionDriver::Submit(
      const OrderInfo& orderInfo) {
    auto order = &*m_orders.insert(std::make_pair(orderInfo.m_orderId,
      std::make_unique<PrimitiveOrder>(orderInfo))).first->second;
    if(m_setOrderStatusNewOnSubmission) {
      SetOrderStatus(*order, OrderStatus::NEW, orderInfo.m_timestamp);
    }
    m_publisher.Push(order);
    return *order;
  }

  inline void MockOrderExecutionDriver::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    auto& order = m_orders.at(orderId);
    order->With(
      [&] (OrderStatus orderStatus,
          const std::vector<ExecutionReport>& executionReports) {
        auto pendingCancelIterator = std::find_if(executionReports.begin(),
          executionReports.end(),
          [&] (const ExecutionReport& executionReport) {
            return executionReport.m_status == OrderStatus::PENDING_CANCEL ||
              IsTerminal(executionReport.m_status);
          });
        if(pendingCancelIterator != executionReports.end()) {
          return;
        }
        auto updatedReport = ExecutionReport::BuildUpdatedReport(
          executionReports.back(), OrderStatus::PENDING_CANCEL,
          boost::posix_time::microsec_clock::universal_time());
        order->Update(updatedReport);
      });
  }

  inline void MockOrderExecutionDriver::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    auto& order = m_orders.at(orderId);
    order->With(
      [&] (OrderStatus orderStatus,
          const std::vector<ExecutionReport>& executionReports) {
        if(IsTerminal(orderStatus)) {
          return;
        }
        auto updatedReport = executionReport;
        updatedReport.m_sequence = executionReports.back().m_sequence + 1;
        if(updatedReport.m_timestamp.is_special()) {
          updatedReport.m_timestamp =
            boost::posix_time::microsec_clock::universal_time();
        }
        order->Update(updatedReport);
      });
  }

  inline void MockOrderExecutionDriver::Close() {}
}
}
}

#endif
