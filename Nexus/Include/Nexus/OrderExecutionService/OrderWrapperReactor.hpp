#ifndef NEXUS_ORDER_REACTOR_HPP
#define NEXUS_ORDER_REACTOR_HPP
#include <type_traits>
#include <Aspen/State.hpp>
#include <Aspen/Traits.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/PublisherReactor.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"

namespace Nexus::OrderExecutionService {

  /** Implements a reactor that wraps an existing Order and evaluates to its
      ExecutionReports.
      @param <C> The type of OrderExecutionClient used to submit the order.
   */
  template<typename C>
  class OrderWrapperReactor {
    public:
      using Type = ExecutionReport;

      /** The type of OrderExecutionClient used to submit the order. */
      using OrderExecutionClient = C;

      /** Constructs an OrderWrapperReactor.
          @param client The OrderExecutionClient used to submit the order.
          @param order The Order to monitor.
       */
      OrderWrapperReactor(Beam::Ref<OrderExecutionClient> client,
        const Order& order);

      /** Cancels the Order. */
      void Cancel();

      Aspen::State commit(int sequence) noexcept;

      Aspen::eval_result_t<Type> eval() const noexcept;

    private:
      OrderExecutionClient* m_client;
      const Order* m_order;
      Beam::Reactors::QueueReactor<ExecutionReport> m_queue;
  };

  template<typename C>
  OrderWrapperReactor<C>::OrderWrapperReactor(
    Beam::Ref<OrderExecutionClient> client, const Order& order)
    : m_client(client.Get()),
      m_order(&order),
      m_queue(Beam::PublisherReactor(m_order->GetPublisher())) {}

  template<typename C>
  void OrderWrapperReactor<C>::Cancel() {
    m_client->Cancel(*m_order);
  }

  template<typename C>
  Aspen::State OrderWrapperReactor<C>::commit(int sequence) noexcept {
    return m_queue.commit(sequence);
  }

  template<typename C>
  const typename OrderWrapperReactor<C>::Type&
      OrderWrapperReactor<C>::eval() const noexcept {
    return m_queue.eval();
  }
}

#endif
