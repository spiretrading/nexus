#ifndef NEXUS_ORDER_WRAPPER_REACTOR_HPP
#define NEXUS_ORDER_WRAPPER_REACTOR_HPP
#include <memory>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/QueueReactor.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /** Implements a reactor that evaluates to an existing Order object. */
  class OrderWrapperReactor {
    public:
      using Type = std::shared_ptr<Order>;

      /**
       * Constructs an OrderWrapperReactor.
       * @param order The order to wrap.
       */
      explicit OrderWrapperReactor(std::shared_ptr<Order> order);

      Aspen::State commit(int sequence) noexcept;

      const std::shared_ptr<Order>& eval() const;

    private:
      std::shared_ptr<Order> m_order;
      std::shared_ptr<Beam::Queue<ExecutionReport>> m_execution_reports;
      Beam::QueueReactor<ExecutionReport> m_queue;
  };

  inline OrderWrapperReactor::OrderWrapperReactor(std::shared_ptr<Order> order)
      : m_order(std::move(order)),
        m_execution_reports(std::make_shared<Beam::Queue<ExecutionReport>>()),
        m_queue(m_execution_reports) {
    m_order->get_publisher().monitor(m_execution_reports);
  }

  inline Aspen::State OrderWrapperReactor::commit(int sequence) noexcept {
    return m_queue.commit(sequence);
  }

  inline const std::shared_ptr<Order>& OrderWrapperReactor::eval() const {
    return m_order;
  }
}

#endif
