#ifndef NEXUS_ORDER_REACTOR_HPP
#define NEXUS_ORDER_REACTOR_HPP
#include <type_traits>
#include <Aspen/State.hpp>
#include <Aspen/Traits.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/PublisherReactor.hpp>
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
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
      using Type = ExecutionReportEntry;

      /** The type of OrderExecutionClient used to submit the order. */
      using OrderExecutionClient = C;

      /** The type of reactor producing OrderFields. */
      using OrderFieldsReactor = O;

      /** Constructs an OrderReactor.
          @param client The OrderExecutionClient used to submit the order.
          @param orderFields The reactor producing OrderFields.
       */
      OrderReactor(Beam::Ref<OrderExecutionClient> client,
        OrderFieldsReactor orderFields);

      /** Cancels the Order. */
      void Cancel();

      Aspen::State commit(int sequence) noexcept;

      const ExecutionReportEntry& eval() const;

    private:
      OrderExecutionClient* m_client;
      OrderFieldsReactor m_orderFields;
      const Order* m_order;
      bool m_hasPendingCancel;
      Beam::Reactors::QueueReactor<ExecutionReport> m_queue;
  };

  template<typename C, typename O>
  OrderReactor<C, O>::OrderReactor(Beam::Ref<OrderExecutionClient> client,
    OrderFieldsReactor orderFields)
    : m_client(client.Get()),
      m_orderFields(std::move(orderFields)),
      m_order(nullptr),
      m_hasPendingCancel(true) {}

  template<typename C, typename O>
  Aspen::State OrderReactor<C, O>::commit(int sequence) noexcept {
    if(m_order == nullptr) {
      auto state = m_orderFields.commit(sequence);
      if(Aspen::has_evaluation(state)) {
        try {
          m_order = &m_client->Submit(m_orderFields.eval());
        } catch(const std::exception&) {
          // TODO
        }
        return Aspen::State::NONE;
      } else {
        return state;
      }
    }
  }

  template<typename C, typename O>
  const ExecutionReportEntry& OrderReactor<C, O>::eval() const {
    return m_queue.eval();
  }
}

#endif
