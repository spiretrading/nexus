#ifndef NEXUS_ORDER_REACTOR_HPP
#define NEXUS_ORDER_REACTOR_HPP
#include <cstdint>
#include <type_traits>
#include <vector>
#include <Aspen/MultiSync.hpp>
#include <Aspen/Sync.hpp>
#include <Aspen/VectorSync.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/MultiQueueReader.hpp>
#include <Beam/Reactors/QueueReactor.hpp>
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
      using Type = const Order*;

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
        std::vector<AdditionalFieldsReactor> additionalFields);

      Aspen::State commit(int sequence) noexcept;

      const Order* eval() const;

    private:
      static constexpr auto NONE = std::uint8_t(0b00000000);
      static constexpr auto FIELDS_COMPLETE = std::uint8_t(0b00000001);
      static constexpr auto FIELDS_AVAILABLE = std::uint8_t(0b00000010);
      static constexpr auto CANCELLING_ORDER = std::uint8_t(0b00000100);
      static constexpr auto WAITING = std::uint8_t(0b00001000);
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
        Aspen::VectorSync<AdditionalFieldsReactor, std::vector<Tag>>>>
        m_orderFields;
      Aspen::Maybe<const Order*> m_order;
      std::uint8_t m_state;
      Quantity m_filled;
      std::shared_ptr<Beam::MultiQueueReader<ExecutionReport>>
        m_executionReports;
      Beam::Reactors::QueueReactor<ExecutionReport> m_queue;
  };

  template<typename C, typename A, typename S, typename R, typename T,
    typename D, typename Q, typename M, typename F>
  auto MakeLimitOrderReactor(Beam::Ref<C> client, A account, S security,
      R currency, T side, D destination, Q quantity, M price, F timeInForce) {
    return OrderReactor(Beam::Ref(client), std::move(account),
      std::move(security), std::move(currency),
      Aspen::constant(OrderType::LIMIT), std::move(side),
      std::move(destination), std::move(quantity), std::move(price),
      std::move(timeInForce), std::vector<Aspen::Constant<Tag>>());
  }

  template<typename C, typename A, typename S, typename R, typename T,
    typename D, typename Q, typename M>
  auto MakeLimitOrderReactor(Beam::Ref<C> client, A account, S security,
      R currency, T side, D destination, Q quantity, M price) {
    return MakeLimitOrderReactor(Beam::Ref(client), std::move(account),
      std::move(security), std::move(currency), std::move(side),
      std::move(destination), std::move(quantity), std::move(price),
      Aspen::constant(TimeInForce(TimeInForce::Type::DAY)));
  }

  template<typename C, typename S, typename T, typename Q, typename M>
  auto MakeLimitOrderReactor(Beam::Ref<C> client, S security, T side,
      Q quantity, M price) {
    return MakeLimitOrderReactor(Beam::Ref(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE()), std::move(side),
      Aspen::constant(std::string()), std::move(quantity), std::move(price));
  }

  template<typename C, typename S, typename T, typename Q, typename M,
    typename F>
  auto MakeLimitOrderReactor(Beam::Ref<C> client, S security, T side,
      Q quantity, M price, F timeInForce) {
    return MakeLimitOrderReactor(Beam::Ref(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE()), std::move(side),
      Aspen::constant(std::string()), std::move(quantity), std::move(price),
      std::move(timeInForce));
  }

  template<typename C, typename S, typename T, typename Q>
  auto MakeMarketOrderReactor(Beam::Ref<C> client, S security, T side,
      Q quantity) {
    return OrderReactor(Beam::Ref(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE()),
      Aspen::constant(OrderType::MARKET), std::move(side),
      Aspen::constant(std::string()), std::move(quantity),
      Aspen::constant(Money::ZERO),
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
    std::vector<AdditionalFieldsReactor> additionalFields)
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
      m_state(NONE),
      m_executionReports(
        std::make_shared<Beam::MultiQueueReader<ExecutionReport>>()),
      m_queue(m_executionReports) {}

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  Aspen::State OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::commit(
      int sequence) noexcept {
    if(m_state & WAITING) {
      auto queueState = m_queue.commit(sequence);
      if(Aspen::has_evaluation(queueState)) {
        auto& report = m_queue.eval();
        m_filled += report.m_lastQuantity;
        if(IsTerminal(report.m_status)) {
          if(m_state & CANCELLING_ORDER ||
              m_filled == m_lastOrderFields->m_quantity) {
            m_state &= ~CANCELLING_ORDER;
            m_state &= ~WAITING;
          } else {
            m_orderFields.reset();
            m_quantity.reset();
            m_order = std::make_exception_ptr(
              std::runtime_error(report.m_text));
            return Aspen::State::COMPLETE_EVALUATED;
          }
        }
      }
      if(Aspen::has_continuation(queueState)) {
        return Aspen::State::CONTINUE;
      }
    }
    if(!(m_state & FIELDS_COMPLETE)) {
      auto fieldsState = m_orderFields->commit(sequence);
      if(Aspen::has_evaluation(fieldsState)) {
        m_state |= FIELDS_AVAILABLE;
      }
      if(Aspen::has_continuation(fieldsState)) {
        return Aspen::State::CONTINUE;
      } else if(Aspen::is_complete(fieldsState) ||
          m_filled == m_lastOrderFields->m_quantity &&
          Aspen::is_complete((*m_quantity)->commit(sequence))) {
        m_state |= FIELDS_COMPLETE;
      }
    }
    if(m_state & FIELDS_AVAILABLE && !(m_state & CANCELLING_ORDER)) {
      if(m_state & WAITING) {
        m_client->Cancel(**m_order);
        m_state |= CANCELLING_ORDER;
      } else {
        try {
          auto orderFields = m_orderFields->eval();
          orderFields.m_quantity -= m_filled;
          if(m_order.has_value() && *m_order == nullptr) {
            m_queue.commit(sequence);
          }
          m_state &= ~FIELDS_AVAILABLE;
          if(m_state & FIELDS_COMPLETE) {
            m_orderFields.reset();
            m_quantity.reset();
          }
          if(orderFields.m_quantity != 0) {
            m_order = &m_client->Submit(orderFields);
            (*m_order)->GetPublisher().Monitor(m_executionReports->GetWriter());
            m_state |= WAITING;
            return Aspen::State::EVALUATED;
          }
        } catch(const std::exception&) {
          m_orderFields.reset();
          m_quantity.reset();
          m_order = std::current_exception();
          return Aspen::State::COMPLETE_EVALUATED;
        }
      }
    } else if(m_state & FIELDS_COMPLETE && !(m_state & WAITING)) {
      return Aspen::State::COMPLETE;
    }
    return Aspen::State::NONE;
  }

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  const Order* OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR,
      RR>::eval() const {
    return *m_order;
  }
}

#endif
