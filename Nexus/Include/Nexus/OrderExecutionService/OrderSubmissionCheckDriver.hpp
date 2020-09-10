#ifndef NEXUS_ORDER_SUBMISSION_CHECK_DRIVER_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_DRIVER_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Performs a series of checks on an Order submission.
   * @param <D> The type of OrderExecutionDriver to send the submission to if
   *        all checks pass.
   */
  template<typename D>
  class OrderSubmissionCheckDriver : private boost::noncopyable {
    public:

      /**
       * The type of OrderExecutionDriver to send the submission to if all
       * checks pass.
       */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an OrderSubmissionCheckDriver.
       * @param orderExecutionDriver The OrderExecutionDriver to send the
       *        submission to if all checks pass.
       * @param orderSubmissionChecks The list of order submission checks to
       *        perform.
       */
      template<typename DF>
      OrderSubmissionCheckDriver(DF&& orderExecutionDriver,
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
      Beam::GetOptionalLocalPtr<D>
        m_orderExecutionDriver;
      Beam::Threading::Sync<std::vector<std::unique_ptr<Order>>> m_orders;
      std::vector<std::unique_ptr<OrderSubmissionCheck>> m_checks;
      Beam::IO::OpenState m_openState;
  };

  template<typename D>
  template<typename DF>
  OrderSubmissionCheckDriver<D>::OrderSubmissionCheckDriver(
    DF&& orderExecutionDriver,
    std::vector<std::unique_ptr<OrderSubmissionCheck>> orderSubmissionChecks)
    : m_orderExecutionDriver(std::forward<DF>(orderExecutionDriver)),
      m_checks(std::move(orderSubmissionChecks)) {}

  template<typename D>
  OrderSubmissionCheckDriver<D>::~OrderSubmissionCheckDriver() {
    Close();
  }

  template<typename D>
  const Order& OrderSubmissionCheckDriver<D>::Recover(
      const SequencedAccountOrderRecord& orderRecord) {
    auto& order = m_orderExecutionDriver->Recover(orderRecord);
    for(auto& check : m_checks) {
      check->Add(order);
    }
    return order;
  }

  template<typename D>
  const Order& OrderSubmissionCheckDriver<D>::Submit(
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
      Beam::Threading::With(m_orders, [&] (auto& orders) {
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

  template<typename D>
  void OrderSubmissionCheckDriver<D>::Cancel(
      const OrderExecutionSession& session, OrderId orderId) {
    return m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename D>
  void OrderSubmissionCheckDriver<D>::Update(
      const OrderExecutionSession& session, OrderId orderId,
      const ExecutionReport& executionReport) {
    return m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename D>
  void OrderSubmissionCheckDriver<D>::Close() {
    m_openState.Close();
  }
}

#endif
