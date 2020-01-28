#ifndef SPIRE_ORDER_PUBLISHER_REACTOR_HPP
#define SPIRE_ORDER_PUBLISHER_REACTOR_HPP
#include <utility>
#include <Aspen/State.hpp>
#include <Beam/Queues/QueueWriter.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /**
   * Takes orders produced by a child reactor and pushes them to a
   * Beam::QueueWriter.
   * @param <P> The type of reactor producing the orders to push.
   */
  template<typename P>
  class OrderPublisherReactor {
    public:
      using Type = const Nexus::OrderExecutionService::Order*;

      /** The type of reactor producing the orders to push. */
      using Producer = P;

      /**
       * Constructs an OrderPublisherReactor.
       * @param queue The queue to push the orders to.
       * @param producer The reactor producing the orders to push.
       */
      OrderPublisherReactor(
        Beam::QueueWriter<const Nexus::OrderExecutionService::Order*>& queue,
        Producer producer);

      Aspen::State commit(int sequence) noexcept;

      const Nexus::OrderExecutionService::Order* eval() const;

    private:
      Beam::QueueWriter<const Nexus::OrderExecutionService::Order*>* m_queue;
      Producer m_producer;
  };

  template<typename P>
  OrderPublisherReactor<P>::OrderPublisherReactor(
    Beam::QueueWriter<const Nexus::OrderExecutionService::Order*>& queue,
    Producer producer)
    : m_queue(&queue),
      m_producer(std::move(producer)) {}

  template<typename P>
  Aspen::State OrderPublisherReactor<P>::commit(int sequence) noexcept {
    auto state = m_producer.commit(sequence);
    if(Aspen::has_evaluation(state)) {
      try {
        m_queue->Push(m_producer.eval());
      } catch(...) {
      }
    }
    return state;
  }

  template<typename P>
  const Nexus::OrderExecutionService::Order*
      OrderPublisherReactor<P>::eval() const {
    return m_producer.eval();
  }
}

#endif
