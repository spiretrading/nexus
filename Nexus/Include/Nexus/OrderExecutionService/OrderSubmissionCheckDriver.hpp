#ifndef NEXUS_ORDER_SUBMISSION_CHECK_DRIVER_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_DRIVER_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Performs a series of checks on an Order submission.
   * @param <D> The type of OrderExecutionDriver to send the submission to if
   *        all checks pass.
   */
  template<IsOrderExecutionDriver D>
  class OrderSubmissionCheckDriver {
    public:

      /**
       * The type of OrderExecutionDriver to send the submission to if all
       * checks pass.
       */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an OrderSubmissionCheckDriver.
       * @param driver The OrderExecutionDriver to send the submission to if all
       *        checks pass.
       * @param checks The list of order submission checks to perform.
       */
      template<Beam::Initializes<D> DF>
      OrderSubmissionCheckDriver(
        DF&& driver, std::vector<std::unique_ptr<OrderSubmissionCheck>> checks);
      ~OrderSubmissionCheckDriver();
      std::shared_ptr<const Order> recover(
        const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<const Order>& order);
      std::shared_ptr<const Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      Beam::GetOptionalLocalPtr<D> m_driver;
      std::vector<std::unique_ptr<OrderSubmissionCheck>> m_checks;
      Beam::IO::OpenState m_open_state;

      OrderSubmissionCheckDriver(const OrderSubmissionCheckDriver&) = delete;
      OrderSubmissionCheckDriver& operator =(
        const OrderSubmissionCheckDriver&) = delete;
  };

  template<typename DF>
  OrderSubmissionCheckDriver(
    DF&&, std::vector<std::unique_ptr<OrderSubmissionCheck>>) ->
      OrderSubmissionCheckDriver<std::remove_reference_t<DF>>;

  template<IsOrderExecutionDriver D>
  template<Beam::Initializes<D> DF>
  OrderSubmissionCheckDriver<D>::OrderSubmissionCheckDriver(
    DF&& driver, std::vector<std::unique_ptr<OrderSubmissionCheck>> checks)
    : m_driver(std::forward<DF>(driver)),
      m_checks(std::move(checks)) {}

  template<IsOrderExecutionDriver D>
  OrderSubmissionCheckDriver<D>::~OrderSubmissionCheckDriver() {
    close();
  }

  template<IsOrderExecutionDriver D>
  std::shared_ptr<const Order> OrderSubmissionCheckDriver<D>::recover(
      const SequencedAccountOrderRecord& record) {
    auto order = m_driver->recover(record);
    for(auto& check : m_checks) {
      check->add(order);
    }
    return order;
  }

  template<IsOrderExecutionDriver D>
  void OrderSubmissionCheckDriver<D>::add(
      const std::shared_ptr<const Order>& order) {
    for(auto& check : m_checks) {
      check->add(order);
    }
    m_driver->add(order);
  }

  template<IsOrderExecutionDriver D>
  std::shared_ptr<const Order> OrderSubmissionCheckDriver<D>::submit(
      const OrderInfo& info) {
    auto submission_iterator = m_checks.begin();
    try {
      while(submission_iterator != m_checks.end()) {
        (*submission_iterator)->submit(info);
        ++submission_iterator;
      }
    } catch(const std::exception& e) {
      for(auto i = m_checks.begin(); i != submission_iterator; ++i) {
        (*i)->reject(info);
      }
      return make_rejected_order(info, e.what());
    }
    auto order = m_driver->submit(info);
    for(auto& check : m_checks) {
      check->add(order);
    }
    return order;
  }

  template<IsOrderExecutionDriver D>
  void OrderSubmissionCheckDriver<D>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    m_driver->cancel(session, id);
  }

  template<IsOrderExecutionDriver D>
  void OrderSubmissionCheckDriver<D>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    return m_driver->update(session, id, report);
  }

  template<IsOrderExecutionDriver D>
  void OrderSubmissionCheckDriver<D>::close() {
    m_open_state.Close();
  }
}

#endif
