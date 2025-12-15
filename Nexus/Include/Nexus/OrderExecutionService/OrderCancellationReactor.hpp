#ifndef NEXUS_ORDER_CANCELLATION_REACTOR_HPP
#define NEXUS_ORDER_CANCELLATION_REACTOR_HPP
#include <atomic>
#include <memory>
#include <Aspen/Aspen.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"

namespace Nexus {

  /**
   * Implements a reactor that cancels all orders produced by a series when
   * that series completes.
   * @param <C> The type of OrderExecutionClient used to cancel the orders.
   * @param <S> The type series producing the orders to cancel.
   */
  template<typename C, typename S> requires
    IsOrderExecutionClient<Beam::dereference_t<C>>
  class OrderCancellationReactor {
    public:

      /** The type of OrderExecutionClient used to cancel the orders. */
      using OrderExecutionClient = Beam::dereference_t<C>;

      /** The type series producing the orders to cancel. */
      using Series = S;

      using Type = std::shared_ptr<Order>;

      static constexpr auto is_noexcept = Aspen::is_noexcept_reactor_v<Series>;

      /**
       * Constructs an OrderCancellationReactor.
       * @param client The OrderExecutionClient used to cancel the order.
       * @param series Produces the series of orders to cancel upon completion.
       */
      template<Beam::Initializes<C> CF>
      OrderCancellationReactor(CF&& client, Series series);

      Aspen::State commit(int sequence) noexcept;
      std::shared_ptr<Order> eval() const noexcept(is_noexcept);

    private:
      Beam::local_ptr_t<C> m_client;
      Series m_series;
      bool m_is_series_complete;
      std::vector<std::shared_ptr<Order>> m_orders;
      std::unique_ptr<std::atomic_int> m_cancel_count;
      Aspen::Trigger* m_trigger;
      std::unique_ptr<Beam::RoutineTaskQueue> m_tasks;
  };

  template<typename C, typename S>
  OrderCancellationReactor(C&&, S) ->
    OrderCancellationReactor<std::remove_cvref_t<C>, S>;

  template<typename C, typename S> requires
    IsOrderExecutionClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  OrderCancellationReactor<C, S>::OrderCancellationReactor(
    CF&& client, Series series)
    : m_client(std::forward<CF>(client)),
      m_series(std::move(series)),
      m_is_series_complete(false),
      m_cancel_count(std::make_unique<std::atomic_int>(0)),
      m_tasks(std::make_unique<Beam::RoutineTaskQueue>()) {}

  template<typename C, typename S> requires
    IsOrderExecutionClient<Beam::dereference_t<C>>
  Aspen::State OrderCancellationReactor<C, S>::commit(int sequence) noexcept {
    if(!m_is_series_complete) {
      auto state = m_series.commit(sequence);
      if(Aspen::has_evaluation(state)) {
        try {
          m_orders.push_back(m_series.eval());
        } catch(...) {}
      }
      if(Aspen::is_complete(state)) {
        m_is_series_complete = true;
        *m_cancel_count = static_cast<int>(m_orders.size());
        if(*m_cancel_count == 0) {
          return state;
        }
        m_trigger = Aspen::Trigger::get_trigger();
        for(auto& order : m_orders) {
          m_client->cancel(order);
          order->get_publisher().monitor(m_tasks->get_slot<ExecutionReport>(
            [cancel_count = m_cancel_count.get(), trigger = m_trigger] (
                const ExecutionReport& report) {
              if(is_terminal(report.m_status)) {
                if(--*cancel_count == 0) {
                  trigger->signal();
                }
              }
            }));
        }
        if(Aspen::has_evaluation(state)) {
          return Aspen::State::EVALUATED;
        }
      } else {
        return state;
      }
    } else if(*m_cancel_count == 0) {
      return Aspen::State::COMPLETE;
    }
    return Aspen::State::NONE;
  }

  template<typename C, typename S> requires
    IsOrderExecutionClient<Beam::dereference_t<C>>
  std::shared_ptr<Order> OrderCancellationReactor<C, S>::eval()
      const noexcept(is_noexcept) {
    return m_series.eval();
  }
}

#endif
