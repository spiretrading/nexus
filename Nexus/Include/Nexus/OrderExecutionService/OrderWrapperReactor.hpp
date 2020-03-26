#ifndef NEXUS_ORDER_WRAPPER_REACTOR_HPP
#define NEXUS_ORDER_WRAPPER_REACTOR_HPP
#include <Aspen/Aspen.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/QueueReactor.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::OrderExecutionService {

  /** Implements a reactor that evaluates to an existing Order object. */
  class OrderWrapperReactor {
    public:
      using Type = const Order*;

      /**
       * Constructs an OrderWrapperReactor.
       * @param order The order to wrap.
       */
      OrderWrapperReactor(Beam::Ref<const Order> order);

      Aspen::State commit(int sequence) noexcept;

      const Order* eval() const;

    private:
      const Order* m_order;
      std::shared_ptr<Beam::Queue<ExecutionReport>> m_executionReports;
      Beam::Reactors::QueueReactor<ExecutionReport> m_queue;
  };

  inline OrderWrapperReactor::OrderWrapperReactor(Beam::Ref<const Order> order)
    : m_order(order.Get()),
      m_executionReports(std::make_shared<Beam::Queue<ExecutionReport>>()),
      m_queue(m_executionReports) {
    m_order->GetPublisher().Monitor(m_executionReports);
  }

  inline Aspen::State OrderWrapperReactor::commit(int sequence) noexcept {
    return m_queue.commit(sequence);
  }

  inline const Order* OrderWrapperReactor::eval() const {
    return m_order;
  }
}

#endif
