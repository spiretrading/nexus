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

  /** Implements a reactor that resubmits Orders when given an updated
      OrderFields.
      @param <C> The type of OrderExecutionClient used to submit the order.
      @param <O> The type of reactor producing OrderFields.
   */
  template<typename C, typename O>
  class OrderReactor {
    public:
      using Type = ExecutionReport;

      /** The type of OrderExecutionClient used to submit the order. */
      using OrderExecutionClient = C;

      /** The type of reactor producing OrderFields. */
      using OrderFieldsReactor = O;

      /** Constructs an OrderReactor.
          @param client The OrderExecutionClient used to submit the order.
          @param orderFields The reactor producing OrderFields.
       */
      template<typename OF>
      OrderReactor(Beam::Ref<OrderExecutionClient> client, OF&& orderFields);

      ~OrderReactor();

      /** Cancels the Order. */
      void Cancel();

      Aspen::State commit(int sequence) noexcept;

      Aspen::eval_result_t<Type> eval() const noexcept;

    private:
      OrderExecutionClient* m_client;
      OrderFieldsReactor m_orderFields;
      const Order* m_order;
      Beam::Reactors::QueueReactor<ExecutionReport> m_queue;
  };

#if 0
  template<typename C, typename O>
  template<typename OF>
  OrderReactor<C, O>::OrderReactor(Beam::Ref<OrderExecutionClient> client,
    OF&& orderFields)
    : m_client(client.Get()),
      m_orderFields(std::forward<OF>(orderFields)),
      m_queue(Beam::PublisherReactor(m_order->GetPublisher())) {}
#endif
}

#endif
