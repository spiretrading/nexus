#ifndef NEXUS_ORDER_REACTOR_HPP
#define NEXUS_ORDER_REACTOR_HPP
#include <cstdint>
#include <type_traits>
#include <vector>
#include <Aspen/MultiSync.hpp>
#include <Aspen/Sync.hpp>
#include <Aspen/VectorSync.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Reactors/QueueReactor.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Implements a reactor that resubmits Orders when given an updated
   * OrderFields.
   * @param <C> The type of OrderExecutionClient used to submit the order.
   * @param <AR> The type of reactor producing the account submitting the
   *             order.
   * @param <SR> The type of reactor producing the order's security.
   * @param <CR> The type of reactor producing the order's currency.
   * @param <OR> The type of reactor producing the order's type.
   * @param <TR> The type of reactor producing the order's side.
   * @param <DR> The type of reactor producing the order's destination.
   * @param <QR> The type of reactor producing the order's quantity.
   * @param <PR> The type of reactor producing the order's price.
   * @param <FR> The type of reactor producing the order's time in force.
   * @param <RR> The type of reactor producing the order's additional fields.
   */
  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  class OrderReactor {
    public:
      using Type = std::shared_ptr<const Order>;

      /** The type of OrderExecutionClient used to submit the order. */
      using OrderExecutionClient = Beam::GetTryDereferenceType<C>;

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
       * @param client The OrderExecutionClient used to submit the order.
       * @param account The type of reactor producing the account field.
       * @param security The type of reactor producing the security field.
       * @param currency The type of reactor producing the currency field.
       * @param order_type The type of reactor producing the order type field.
       * @param side The type of reactor producing the side field.
       * @param destination The type of reactor producing the destination
       *        field.
       * @param quantity The type of reactor producing the quantity field.
       * @param price The type of reactor producing the price field.
       * @param time_in_force The type of reactor producing the time in force
       *        field.
       * @param additional_fields The type of reactor producing the additional
       *        fields.
       */
      template<typename CF>
      OrderReactor(CF&& client, AccountReactor account,
        SecurityReactor security, CurrencyReactor currency,
        OrderTypeReactor order_type, SideReactor side,
        DestinationReactor destination, QuantityReactor quantity,
        PriceReactor price, TimeInForceReactor time_in_force,
        std::vector<AdditionalFieldsReactor> additional_fields);

      Aspen::State commit(int sequence) noexcept;

      std::shared_ptr<const Order> eval() const;

    private:
      static constexpr auto NONE = std::uint8_t(0b00000000);
      static constexpr auto FIELDS_COMPLETE = std::uint8_t(0b00000001);
      static constexpr auto FIELDS_AVAILABLE = std::uint8_t(0b00000010);
      static constexpr auto CANCELLING_ORDER = std::uint8_t(0b00000100);
      static constexpr auto WAITING = std::uint8_t(0b00001000);
      Beam::GetOptionalLocalPtr<C> m_client;
      std::unique_ptr<OrderFields> m_last_order_fields;
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
          m_order_fields;
      Aspen::Maybe<std::shared_ptr<const Order>> m_order;
      std::uint8_t m_state;
      Quantity m_filled;
      std::shared_ptr<Beam::MultiQueueWriter<ExecutionReport>>
        m_execution_reports;
      Beam::Reactors::QueueReactor<ExecutionReport> m_queue;
  };

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  OrderReactor(C&&, AR, SR, CR, OR, TR, DR, QR, PR, FR, std::vector<RR>) ->
    OrderReactor<std::decay_t<C>, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>;

  template<typename C, typename A, typename S, typename R, typename T,
    typename D, typename Q, typename M, typename F>
  auto make_limit_order_reactor(C&& client, A account, S security, R currency,
      T side, D destination, Q quantity, M price, F time_in_force) {
    return OrderReactor(std::forward<C>(client), std::move(account),
      std::move(security), std::move(currency),
      Aspen::constant(OrderType::LIMIT), std::move(side),
      std::move(destination), std::move(quantity), std::move(price),
      std::move(time_in_force), std::vector<Aspen::Constant<Tag>>());
  }

  template<typename C, typename A, typename S, typename R, typename T,
    typename D, typename Q, typename M>
  auto make_limit_order_reactor(C&& client, A account, S security, R currency,
      T side, D destination, Q quantity, M price) {
    return make_limit_order_reactor(std::forward<C>(client), std::move(account),
      std::move(security), std::move(currency), std::move(side),
      std::move(destination), std::move(quantity), std::move(price),
      Aspen::constant(TimeInForce(TimeInForce::Type::DAY)));
  }

  template<typename C, typename S, typename T, typename Q, typename M>
  auto make_limit_order_reactor(
      C&& client, S security, T side, Q quantity, M price) {
    return make_limit_order_reactor(std::forward<C>(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE), std::move(side),
      Aspen::constant(std::string()), std::move(quantity), std::move(price));
  }

  template<typename C, typename S, typename T, typename Q, typename M,
    typename F>
  auto make_limit_order_reactor(
      C&& client, S security, T side, Q quantity, M price, F time_in_force) {
    return make_limit_order_reactor(std::forward<C>(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE), std::move(side),
      Aspen::constant(std::string()), std::move(quantity), std::move(price),
      std::move(time_in_force));
  }

  template<typename C, typename S, typename T, typename Q>
  auto make_market_order_reactor(C&& client, S security, T side, Q quantity) {
    return OrderReactor(std::forward<C>(client),
      Aspen::constant(Beam::ServiceLocator::DirectoryEntry()),
      std::move(security), Aspen::constant(CurrencyId::NONE),
      Aspen::constant(OrderType::MARKET), std::move(side),
      Aspen::constant(std::string()), std::move(quantity),
      Aspen::constant(Money::ZERO),
      Aspen::constant(TimeInForce(TimeInForce::Type::DAY)),
      std::vector<Aspen::Constant<Tag>>());
  }

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  template<typename CF>
  OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::OrderReactor(
    CF&& client, AccountReactor account, SecurityReactor security,
    CurrencyReactor currency, OrderTypeReactor order_type, SideReactor side,
    DestinationReactor destination, QuantityReactor quantity,
    PriceReactor price, TimeInForceReactor time_in_force,
    std::vector<AdditionalFieldsReactor> additional_fields)
    : m_client(std::forward<CF>(client)),
      m_quantity(std::move(quantity)),
      m_last_order_fields(std::make_unique<OrderFields>()),
      m_order_fields(std::in_place, *m_last_order_fields,
        Aspen::Sync(m_last_order_fields->m_account, std::move(account)),
        Aspen::Sync(m_last_order_fields->m_security, std::move(security)),
        Aspen::Sync(m_last_order_fields->m_currency, std::move(currency)),
        Aspen::Sync(m_last_order_fields->m_type, std::move(order_type)),
        Aspen::Sync(m_last_order_fields->m_side, std::move(side)),
        Aspen::Sync(m_last_order_fields->m_destination, std::move(destination)),
        Aspen::Sync(m_last_order_fields->m_quantity, *m_quantity),
        Aspen::Sync(m_last_order_fields->m_price, std::move(price)),
        Aspen::Sync(
          m_last_order_fields->m_time_in_force, std::move(time_in_force)),
        Aspen::VectorSync(m_last_order_fields->m_additional_fields,
          std::move(additional_fields))),
      m_order(std::shared_ptr<const Order>()),
      m_state(NONE),
      m_execution_reports(
        std::make_shared<Beam::MultiQueueWriter<ExecutionReport>>()),
      m_queue(m_execution_reports) {}

  template<typename C, typename AR, typename SR, typename CR, typename OR,
    typename TR, typename DR, typename QR, typename PR, typename FR,
    typename RR>
  Aspen::State OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::commit(
      int sequence) noexcept {
    if(m_state & WAITING) {
      auto queue_state = m_queue.commit(sequence);
      if(Aspen::has_evaluation(queue_state)) {
        auto& report = m_queue.eval();
        m_filled += report.m_last_quantity;
        if(is_terminal(report.m_status)) {
          if(m_state & CANCELLING_ORDER ||
              m_filled == m_last_order_fields->m_quantity) {
            m_state &= ~CANCELLING_ORDER;
            m_state &= ~WAITING;
          } else {
            m_order_fields.reset();
            m_quantity.reset();
            m_order =
              std::make_exception_ptr(std::runtime_error(report.m_text));
            return Aspen::State::COMPLETE_EVALUATED;
          }
        }
      }
      if(Aspen::has_continuation(queue_state)) {
        return Aspen::State::CONTINUE;
      }
    }
    if(!(m_state & FIELDS_COMPLETE)) {
      auto fields_state = m_order_fields->commit(sequence);
      if(Aspen::has_evaluation(fields_state)) {
        m_state |= FIELDS_AVAILABLE;
      }
      if(Aspen::has_continuation(fields_state)) {
        return Aspen::State::CONTINUE;
      } else if(Aspen::is_complete(fields_state) ||
          m_filled == m_last_order_fields->m_quantity &&
            Aspen::is_complete((*m_quantity)->commit(sequence))) {
        m_state |= FIELDS_COMPLETE;
      }
    }
    if(m_state & FIELDS_AVAILABLE && !(m_state & CANCELLING_ORDER)) {
      if(m_state & WAITING) {
        m_client->cancel(*m_order);
        m_state |= CANCELLING_ORDER;
      } else {
        try {
          auto fields = m_order_fields->eval();
          fields.m_quantity -= m_filled;
          if(m_order.has_value() && *m_order == nullptr) {
            m_queue.commit(sequence);
          }
          m_state &= ~FIELDS_AVAILABLE;
          if(m_state & FIELDS_COMPLETE) {
            m_order_fields.reset();
            m_quantity.reset();
          }
          if(fields.m_quantity != 0) {
            m_order = m_client->submit(fields);
            (*m_order)->get_publisher().Monitor(
              m_execution_reports->GetWriter());
            m_state |= WAITING;
            return Aspen::State::EVALUATED;
          } else if(m_state & FIELDS_COMPLETE && !(m_state & WAITING)) {
            return Aspen::State::COMPLETE;
          }
        } catch(const std::exception&) {
          m_order_fields.reset();
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
  std::shared_ptr<const Order>
      OrderReactor<C, AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::eval() const {
    return *m_order;
  }
}

#endif
