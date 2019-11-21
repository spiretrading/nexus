#ifndef NEXUS_ORDER_REACTOR_HPP
#define NEXUS_ORDER_REACTOR_HPP
#include <type_traits>
#include <Aspen/MultiSync.hpp>
#include <Aspen/Sync.hpp>
#include <Aspen/VectorSync.hpp>
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
      @param <AR> The type of reactor producing the account submitting the
                  order.
      @param <SR> The type of reactor producing the order's security.
      @param <CR> The type of reactor producing the order's currency.
      @param <OR> The type of reactor producing the order's type.
      @param <TR> The type of reactor producing the order's side.
      @param <DR> The type of reactor producing the order's destination.
      @param <QR> The type of reactor producing the order's quantity.
      @param <PR> The type of reactor producing the order's price.
      @param <FR> The type of reactor producing the order's time in force.
      @param <RR> The type of reactor producing the order's additional fields.
   */
  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  class OrderReactor {
    public:
      using Type = ExecutionReportEntry;

      /** The type of OrderExecutionClient used to submit the order. */
      using OrderExecutionClient = C;

      /** The type of reactor producing the account submitting the order. */
      using AccountReactor = AR;

      /** The type of reactor producing the order's security. */
      using SecurityReactor = SR;

      /** The type of reactor producing the order's currency. */
      using CurrencyReactor = CR;

      /** The type of reactor producing the order's type. */
      using OrderTypeReactor = OR;

      /** The type of reactor producing the order's side. */
      using SideReactor = TR;

      /** The type of reactor producing the order's destination. */
      using DestinationReactor = DR;

      /** The type of reactor producing the order's quantity. */
      using QuantityReactor = QR;

      /** The type of reactor producing the order's price. */
      using PriceReactor = PR;

      /** The type of reactor producing the order's time in force. */
      using TimeInForceReactor = FR;

      /** The type of reactor producing the order's additional fields. */
      using AdditionalFieldsReactor = RR;

      /** Constructs an OrderReactor.
          @param client The OrderExecutionClient used to submit the order.
          @param account The type of reactor producing the account field.
          @param security The type of reactor producing the security field.
          @param currency The type of reactor producing the currency field.
          @param orderType The type of reactor producing the order type field.
          @param side The type of reactor producing the side field.
          @param destination The type of reactor producing the destination
                 field.
          @param quantity The type of reactor producing the quantity field.
          @param price The type of reactor producing the price field.
          @param timeInForce The type of reactor producing the time in force
                 field.
          @param additionalFields The type of reactor producing the additional
                 fields.
       */
      OrderReactor(Beam::Ref<OrderExecutionClient> client,
        AccountReactor account, SecurityReactor security,
        CurrencyReactor currency, OrderTypeReactor orderType, SideReactor side,
        DestinationReactor destination, QuantityReactor quantity,
        PriceReactor price, TimeInForceReactor timeInForce,
        AdditionalFieldsReactor additionalFields);

      /** Cancels the Order. */
      void Cancel();

      Aspen::State commit(int sequence) noexcept;

      const ExecutionReportEntry& eval() const;

    private:
      OrderExecutionClient* m_client;
      std::unique_ptr<OrderFields> m_lastOrderFields;
      std::optional<Aspen::Shared<QuantityReactor>> m_quantity;
      std::optional<Aspen::MultiSync<OrderFields,
        Aspen::Sync<AccountReactor, Beam::ServiceLocator::DirectoryEntry>,
        Aspen::Sync<SecurityReactor, Security>,
        Aspen::Sync<CurrencyReactor, CurrencyId>,
        Aspen::Sync<OrderTypeReactor, OrderType>,
        Aspen::Sync<SideReactor, Side>,
        Aspen::Sync<DestinationReactor, Destination>,
        Aspen::Sync<Aspen::Shared<QuantityReactor>, Quantity>,
        Aspen::Sync<PriceReactor, Money>,
        Aspen::Sync<TimeInForceReactor, TimeInForce>,
        Aspen::VectorSync<typename AdditionalFieldsReactor::value_type,
        std::vector<Tag>>>> m_orderFields;
      const Order* m_order;
      bool m_isPendingCancel;
      Quantity m_filled;
      OrderStatus m_status;
      std::shared_ptr<Beam::MultiQueueReader<ExecutionReportEntry>>
        m_executionReports;
      std::shared_ptr<Beam::QueueWriter<ExecutionReport>> m_orderQueue;
      Beam::Reactors::QueueReactor<ExecutionReportEntry> m_queue;
  };

  template<typename C, typename S, typename T, typename Q, typename M>
  auto MakeLimitOrderReactor(Beam::Ref<C> client, S security, T side,
      Q quantity, M price) {
    return OrderReactor(Beam::Ref(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE()),
      Aspen::constant(OrderType::LIMIT), std::move(side),
      Aspen::constant(std::string()), std::move(quantity), std::move(price),
      Aspen::constant(TimeInForce(TimeInForce::Type::DAY)),
      std::vector<Aspen::Constant<Tag>>());
  }

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::OrderReactor(
    Beam::Ref<OrderExecutionClient> client, AccountReactor account,
    SecurityReactor security, CurrencyReactor currency,
    OrderTypeReactor orderType, SideReactor side,
    DestinationReactor destination, QuantityReactor quantity,
    PriceReactor price, TimeInForceReactor timeInForce,
    AdditionalFieldsReactor additionalFields)
    : m_client(client.Get()),
      m_quantity(std::move(quantity)),
      m_lastOrderFields(std::make_unique<OrderFields>()),
      m_orderFields(std::in_place, *m_lastOrderFields,
        Aspen::Sync(m_lastOrderFields->m_account, std::move(account)),
        Aspen::Sync(m_lastOrderFields->m_security, std::move(security)),
        Aspen::Sync(m_lastOrderFields->m_currency, std::move(currency)),
        Aspen::Sync(m_lastOrderFields->m_type, std::move(orderType)),
        Aspen::Sync(m_lastOrderFields->m_side, std::move(side)),
        Aspen::Sync(m_lastOrderFields->m_destination, std::move(destination)),
        Aspen::Sync(m_lastOrderFields->m_quantity, *m_quantity),
        Aspen::Sync(m_lastOrderFields->m_price, std::move(price)),
        Aspen::Sync(m_lastOrderFields->m_timeInForce, std::move(timeInForce)),
        Aspen::VectorSync(m_lastOrderFields->m_additionalFields,
          std::move(additionalFields))),
      m_order(nullptr),
      m_isPendingCancel(true),
      m_status(OrderStatus::CANCELED),
      m_executionReports(
        std::make_shared<Beam::MultiQueueReader<ExecutionReportEntry>>()),
      m_queue(m_executionReports) {}

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  Aspen::State OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::commit(
      int sequence) noexcept {
    auto queueState = m_queue.commit(sequence);
    if(Aspen::has_evaluation(queueState)) {
      const auto& report = m_queue.eval().m_executionReport;
      m_status = report.m_status;
      m_filled += report.m_lastQuantity;
      if(!IsTerminal(m_status)) {
        if(Aspen::has_continuation(queueState)) {
          return Aspen::State::CONTINUE_EVALUATED;
        }
        return Aspen::State::EVALUATED;
      } else if(!m_isPendingCancel && !m_orderFields.has_value()) {
        return Aspen::State::COMPLETE_EVALUATED;
      }
    } else if(Aspen::has_continuation(queueState)) {
      return Aspen::State::CONTINUE;
    }
    if(m_orderFields.has_value()) {
      auto fieldsState = m_orderFields->commit(sequence);
      if(Aspen::has_evaluation(fieldsState) && !m_isPendingCancel &&
          !IsTerminal(m_status)) {
        m_client->Cancel(*m_order);
        m_isPendingCancel = true;
      }
      if(m_filled == (*m_quantity)->eval() &&
          Aspen::is_complete((*m_quantity)->commit(sequence)) ||
          Aspen::is_complete(fieldsState)) {
        m_orderFields.reset();
        m_quantity.reset();
        if(m_order == nullptr && !Aspen::has_evaluation(fieldsState)) {
          return Aspen::State::COMPLETE;
        }
      } else if(Aspen::has_continuation(fieldsState)) {
        return Aspen::combine(queueState, Aspen::State::CONTINUE);
      }
    }
    if(IsTerminal(m_status)) {
      try {
        m_isPendingCancel = false;
        if(m_lastOrderFields->m_quantity - m_filled > 0) {
          auto orderFields = m_orderFields->eval();
          orderFields.m_quantity -= m_filled;
          m_order = &m_client->Submit(m_orderFields->eval());
          m_orderQueue = Beam::MakeConverterWriterQueue<ExecutionReport>(
            m_executionReports->GetWriter(),
            [order = m_order] (const auto& executionReport) {
              return ExecutionReportEntry{order, executionReport};
            });
          m_order->GetPublisher().Monitor(m_orderQueue);
        }
      } catch(const std::exception&) {
        return Aspen::State::COMPLETE;
      }
      return queueState;
    }
    return Aspen::State::NONE;
  }

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  const ExecutionReportEntry& OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR,
      FR, RR>::eval() const {
    return m_queue.eval();
  }
}

#endif
