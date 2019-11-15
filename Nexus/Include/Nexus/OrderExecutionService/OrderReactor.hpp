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
      std::optional<AccountReactor> m_account;
      std::optional<SecurityReactor> m_security;
      std::optional<CurrencyReactor> m_currency;
      std::optional<OrderTypeReactor> m_orderType;
      std::optional<SideReactor> m_side;
      std::optional<DestinationReactor> m_destination;
      std::optional<Aspen::collapse_shared<QuantityReactor>> m_quantity;
      std::optional<PriceReactor> m_price;
      std::optional<TimeInForceReactor> m_timeInForce;
      std::optional<AdditionalFieldsReactor> m_additionalFields;
      Aspen::Maybe<OrderFields> m_lastOrderFields;
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
      Aspen::constant(std::vector<Tag>()));
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
      m_orderFields(std::move(account), std::move(security),
        std::move(currency), std::move(orderType), std::move(side),
        std::move(destination), m_quantity, std::move(price),
        std::move(timeInForce), std::move(additionalFields)),
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
      if(Aspen::has_evaluation(fieldsState)) {
        m_lastOrderFields = Aspen::try_call(
          [&] {
            if(m_filled == 0) {
              return m_orderFields->eval();
            }
            auto modifiedFields = m_orderFields->eval();
            modifiedFields.m_quantity -= m_filled;
            return modifiedFields;
          });
        if(!m_isPendingCancel && !IsTerminal(m_status)) {
          m_client->Cancel(*m_order);
          m_isPendingCancel = true;
        }
      }
      if(Aspen::has_continuation(fieldsState)) {
        return Aspen::combine(queueState, Aspen::State::CONTINUE);
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
        const auto& orderFields = *m_lastOrderFields;
        if(orderFields.m_quantity > 0) {
          m_order = &m_client->Submit(orderFields);
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
