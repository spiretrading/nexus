#ifndef NEXUS_ORDERSUBMISSIONCHECKDRIVER_HPP
#define NEXUS_ORDERSUBMISSIONCHECKDRIVER_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class OrderSubmissionCheckDriver
      \brief Performs a series of checks on an Order submission.
      \tparam OrderExecutionDriverType The type of OrderExecutionDriver to send
              the submission to if all checks pass.
   */
  template<typename OrderExecutionDriverType>
  class OrderSubmissionCheckDriver : private boost::noncopyable {
    public:

      //! The type of OrderExecutionDriver to send the submission to if all
      //! checks pass.
      using OrderExecutionDriver =
        Beam::GetTryDereferenceType<OrderExecutionDriverType>;

      //! Constructs an OrderSubmissionCheckDriver.
      /*!
        \param orderExecutionDriver The OrderExecutionDriver to send the
               submission to if all checks pass.
        \param orderSubmissionChecks The list of order submission checks to
               perform.
      */
      template<typename OrderExecutionDriverForward>
      OrderSubmissionCheckDriver(
        OrderExecutionDriverForward&& orderExecutionDriver,
        std::vector<std::unique_ptr<OrderSubmissionCheck>>
        orderSubmissionChecks);

      ~OrderSubmissionCheckDriver();

      const Order& Recover(const SequencedAccountOrderRecord& order);

      const Order& Submit(const OrderInfo& orderInfo);

      void Cancel(const OrderExecutionSession& session, OrderId orderId);

      void Update(const OrderExecutionSession& session, OrderId orderId,
        const ExecutionReport& executionReport);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<OrderExecutionDriverType>
        m_orderExecutionDriver;
      Beam::Threading::Sync<std::vector<std::unique_ptr<Order>>> m_orders;
      std::vector<std::unique_ptr<OrderSubmissionCheck>> m_checks;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename OrderExecutionDriverType>
  template<typename OrderExecutionDriverForward>
  OrderSubmissionCheckDriver<OrderExecutionDriverType>::
      OrderSubmissionCheckDriver(OrderExecutionDriverForward&&
      orderExecutionDriver, std::vector<std::unique_ptr<OrderSubmissionCheck>>
      orderSubmissionChecks)
      : m_orderExecutionDriver(std::forward<OrderExecutionDriverForward>(
          orderExecutionDriver)),
        m_checks(std::move(orderSubmissionChecks)) {
    m_openState.SetOpen();
  }

  template<typename OrderExecutionDriverType>
  OrderSubmissionCheckDriver<OrderExecutionDriverType>::
      ~OrderSubmissionCheckDriver() {
    Close();
  }

  template<typename OrderExecutionDriverType>
  const Order& OrderSubmissionCheckDriver<OrderExecutionDriverType>::Recover(
      const SequencedAccountOrderRecord& orderRecord) {
    auto& order = m_orderExecutionDriver->Recover(orderRecord);
    for(auto& check : m_checks) {
      check->Add(order);
    }
    return order;
  }

  template<typename OrderExecutionDriverType>
  const Order& OrderSubmissionCheckDriver<OrderExecutionDriverType>::Submit(
      const OrderInfo& orderInfo) {
    auto submissionIterator = m_checks.begin();
    try {
      while(submissionIterator != m_checks.end()) {
        (*submissionIterator)->Submit(orderInfo);
        ++submissionIterator;
      }
    } catch(const std::exception& e) {
      for(auto i = m_checks.begin(); i != submissionIterator; ++i) {
        (*i)->Reject(orderInfo);
      }
      auto order = BuildRejectedOrder(orderInfo, e.what());
      auto result = order.get();
      Beam::Threading::With(m_orders,
        [&] (std::vector<std::unique_ptr<Order>>& orders) {
          orders.emplace_back(std::move(order));
        });
      return *result;
    }
    auto& order = m_orderExecutionDriver->Submit(orderInfo);
    for(auto& check : m_checks) {
      check->Add(order);
    }
    return order;
  }

  template<typename OrderExecutionDriverType>
  void OrderSubmissionCheckDriver<OrderExecutionDriverType>::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    return m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename OrderExecutionDriverType>
  void OrderSubmissionCheckDriver<OrderExecutionDriverType>::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    return m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename OrderExecutionDriverType>
  void OrderSubmissionCheckDriver<OrderExecutionDriverType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename OrderExecutionDriverType>
  void OrderSubmissionCheckDriver<OrderExecutionDriverType>::Shutdown() {
    m_openState.SetClosed();
  }
}
}

#endif
