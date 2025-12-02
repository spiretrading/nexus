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
  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  class OrderSubmissionCheckDriver {
    public:

      /**
       * The type of OrderExecutionDriver to send the submission to if all
       * checks pass.
       */
      using OrderExecutionDriver = Beam::dereference_t<D>;

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

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      Beam::local_ptr_t<D> m_driver;
      std::vector<std::unique_ptr<OrderSubmissionCheck>> m_checks;
      Beam::OpenState m_open_state;

      OrderSubmissionCheckDriver(const OrderSubmissionCheckDriver&) = delete;
      OrderSubmissionCheckDriver& operator =(
        const OrderSubmissionCheckDriver&) = delete;
  };

  template<typename D>
  OrderSubmissionCheckDriver(
    D&&, std::vector<std::unique_ptr<OrderSubmissionCheck>>) ->
      OrderSubmissionCheckDriver<std::remove_cvref_t<D>>;

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  OrderSubmissionCheckDriver<D>::OrderSubmissionCheckDriver(
    DF&& driver, std::vector<std::unique_ptr<OrderSubmissionCheck>> checks)
    : m_driver(std::forward<DF>(driver)),
      m_checks(std::move(checks)) {}

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  OrderSubmissionCheckDriver<D>::~OrderSubmissionCheckDriver() {
    close();
  }

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  std::shared_ptr<Order> OrderSubmissionCheckDriver<D>::recover(
      const SequencedAccountOrderRecord& record) {
    auto order = m_driver->recover(record);
    for(auto& check : m_checks) {
      check->add(order);
    }
    return order;
  }

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  void OrderSubmissionCheckDriver<D>::add(const std::shared_ptr<Order>& order) {
    for(auto& check : m_checks) {
      check->add(order);
    }
    m_driver->add(order);
  }

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  std::shared_ptr<Order> OrderSubmissionCheckDriver<D>::submit(
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

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  void OrderSubmissionCheckDriver<D>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    m_driver->cancel(session, id);
  }

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  void OrderSubmissionCheckDriver<D>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    return m_driver->update(session, id, report);
  }

  template<typename D> requires IsOrderExecutionDriver<Beam::dereference_t<D>>
  void OrderSubmissionCheckDriver<D>::close() {
    m_open_state.close();
  }
}

#endif
