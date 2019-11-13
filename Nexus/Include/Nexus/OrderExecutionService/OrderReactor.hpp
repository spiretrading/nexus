#ifndef NEXUS_ORDER_REACTOR_HPP
#define NEXUS_ORDER_REACTOR_HPP
#include <optional>
#include <type_traits>
#include <Aspen/State.hpp>
#include <Aspen/Traits.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/ConverterWriterQueue.hpp>
#include <Beam/Queues/MultiQueueReader.hpp>
#include <Beam/Reactors/QueueReactor.hpp>
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
      std::optional<OrderFieldsReactor> m_orderFields;
      Aspen::Maybe<OrderFields> m_lastOrderFields;
      const Order* m_order;
      bool m_isPendingCancel;
      OrderStatus m_status;
      std::shared_ptr<Beam::MultiQueueReader<ExecutionReportEntry>>
        m_executionReports;
      Beam::Reactors::QueueReactor<ExecutionReportEntry> m_queue;
  };

  template<typename C, typename O>
  OrderReactor<C, O>::OrderReactor(Beam::Ref<OrderExecutionClient> client,
    OrderFieldsReactor orderFields)
    : m_client(client.Get()),
      m_orderFields(std::move(orderFields)),
      m_order(nullptr),
      m_isPendingCancel(true),
      m_status(OrderStatus::CANCELED),
      m_executionReports(
        std::make_shared<Beam::MultiQueueReader<ExecutionReportEntry>>()),
      m_queue(m_executionReports) {}

  template<typename C, typename O>
  Aspen::State OrderReactor<C, O>::commit(int sequence) noexcept {
    auto queueState = m_queue.commit(sequence);
    if(queueState != Aspen::State::NONE) {
      if(Aspen::has_evaluation(queueState)) {
        m_status = m_queue.eval().m_executionReport.m_status;
        if(IsTerminal(m_status) && !m_orderFields.has_value()) {
          return Aspen::State::COMPLETE_EVALUATED;
        }
      }
      if(Aspen::is_complete(queueState)) {
        return queueState;
      }
      return Aspen::State::CONTINUE;
    }
    if(m_orderFields.has_value()) {
      auto fieldsState = m_orderFields->commit(sequence);
      if(Aspen::has_evaluation(fieldsState)) {
        m_lastOrderFields = Aspen::try_eval(*m_orderFields);
        if(!m_isPendingCancel) {
          m_client->Cancel(*m_order);
          m_isPendingCancel = true;
        }
      }
      if(Aspen::has_continuation(fieldsState)) {
        return Aspen::State::CONTINUE;
      } else if(Aspen::is_complete(fieldsState)) {
        m_orderFields.reset();
        if(m_order == nullptr && !Aspen::has_evaluation(fieldsState)) {
          return Aspen::State::COMPLETE;
        }
      }
    }
    if(IsTerminal(m_status)) {
      try {
        m_isPendingCancel = false;
        m_order = &m_client->Submit(*m_lastOrderFields);
        m_order->GetPublisher().Monitor(
          Beam::MakeConverterWriterQueue<ExecutionReport>(
            m_executionReports->GetWriter(),
          [order = m_order] (const auto& executionReport) {
            return ExecutionReportEntry{order, executionReport};
          }));
      } catch(const std::exception&) {
        // TODO
      }
    }
    return Aspen::State::NONE;
  }

  template<typename C, typename O>
  const ExecutionReportEntry& OrderReactor<C, O>::eval() const {
    return m_queue.eval();
  }
}

#endif
