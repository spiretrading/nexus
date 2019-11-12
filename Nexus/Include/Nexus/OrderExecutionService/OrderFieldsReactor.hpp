#ifndef NEXUS_ORDER_FIELDS_REACTOR_HPP
#define NEXUS_ORDER_FIELDS_REACTOR_HPP
#include <exception>
#include <optional>
#include <Aspen/Aspen.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::OrderExecutionService  {
  template<typename AR, typename SR, typename CR, typename OR, typename TR,
    typename DR, typename QR, typename PR, typename FR, typename RR>
  class OrderFieldsReactor {
    public:
      using Type = OrderFields;

      using AccountReactor = AR;

      using SecurityReactor = SR;

      using CurrencyReactor = CR;

      using OrderTypeReactor = OR;

      using SideReactor = TR;

      using DestinationReactor = DR;

      using QuantityReactor = QR;

      using PriceReactor = PR;

      using TimeInForceReactor = FR;

      using AdditionalFieldsReactor = RR;

      OrderFieldsReactor(AccountReactor account, SecurityReactor security,
        CurrencyReactor currency, OrderTypeReactor orderType, SideReactor side,
        DestinationReactor destination, QuantityReactor quantity,
        PriceReactor price, TimeInForceReactor timeInForce,
        AdditionalFieldsReactor additionalFields);

      Aspen::State commit(int sequence) noexcept;

      const OrderFields& eval() const;

    private:
      std::optional<AccountReactor> m_account;
      std::optional<SecurityReactor> m_security;
      std::optional<CurrencyReactor> m_currency;
      std::optional<OrderTypeReactor> m_orderType;
      std::optional<SideReactor> m_side;
      std::optional<DestinationReactor> m_destination;
      std::optional<QuantityReactor> m_quantity;
      std::optional<PriceReactor> m_price;
      std::optional<TimeInForceReactor> m_timeInForce;
      std::optional<AdditionalFieldsReactor> m_additionalFields;
      OrderFields m_orderFields;
      std::exception_ptr m_exception;

      template<typename F, typename R>
      void UpdateField(Aspen::State& state, F& field, R& reactor, int sequence,
        int completionCount);
  };

  template<typename AR, typename SR, typename CR, typename OR, typename TR,
    typename DR, typename QR, typename PR, typename FR, typename RR>
  OrderFieldsReactor<AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::
    OrderFieldsReactor(AccountReactor account, SecurityReactor security,
    CurrencyReactor currency, OrderTypeReactor orderType, SideReactor side,
    DestinationReactor destination, QuantityReactor quantity,
    PriceReactor price, TimeInForceReactor timeInForce,
    AdditionalFieldsReactor additionalFields)
    : m_account(std::move(account)),
      m_security(std::move(security)),
      m_currency(std::move(currency)),
      m_orderType(std::move(orderType)),
      m_side(std::move(side)),
      m_destination(std::move(destination)),
      m_quantity(std::move(quantity)),
      m_price(std::move(price)),
      m_timeInForce(std::move(timeInForce)),
      m_additionalFields(std::move(additionalFields)) {}

  template<typename AR, typename SR, typename CR, typename OR, typename TR,
    typename DR, typename QR, typename PR, typename FR, typename RR>
  Aspen::State OrderFieldsReactor<AR, SR, CR, OR, TR, DR, QR, PR, FR,
      RR>::commit(int sequence) noexcept {
    constexpr REACTOR_COUNT = 10;
    auto previousException = m_exception;
    m_exception = nullptr;
    auto state = Aspen::State::NONE;
    auto completionCount = 0;
    UpdateField(state, m_orderFields.m_account, m_account, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_security, m_security, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_currency, m_currency, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_type, m_orderType, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_side, m_side, sequence, completionCount);
    UpdateField(state, m_orderFields.m_destination, m_destination, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_quantity, m_quantity, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_price, m_price, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_timeInForce, m_timeInForce, sequence,
      completionCount);
    UpdateField(state, m_orderFields.m_additionalFields, m_additionalFields,
      sequence, completionCount);
    if(!Aspen::has_evaluation(state)) {
      m_exception = previousException;
    }
    if(completionCount == REACTOR_COUNT) {
      state = Aspen::combine(state, Aspen::State::COMPLETE);
    }
    return state;
  }

  template<typename AR, typename SR, typename CR, typename OR, typename TR,
    typename DR, typename QR, typename PR, typename FR, typename RR>
  const OrderFields& OrderFieldsReactor<AR, SR, CR, OR, TR, DR, QR, PR, FR,
      RR>::eval() const {
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
    return m_orderFields;
  }

  template<typename AR, typename SR, typename CR, typename OR, typename TR,
    typename DR, typename QR, typename PR, typename FR, typename RR>
  template<typename F, typename R>
  void OrderFieldsReactor<AR, SR, CR, OR, TR, DR, QR, PR, FR, RR>::UpdateField(
      Aspen::State& state, F& field, R& reactor, int sequence,
      int completionCount) {
    if(reactor.has_value()) {
      auto fieldState = reactor->commit(sequence);
      if(Aspen::has_evaluation(fieldState)) {
        try {
          field = reactor->eval();
        } catch(const std::exception&) {
          m_exception = std::current_exception();
        }
        state = Aspen::combine(state, Aspen::State::EVALUATED);
      }
      if(Aspen::is_complete(fieldState)) {
        reactor.reset();
        ++completionCount;
      } else if(Aspen::has_continuation(fieldState)) {
        state = Aspen::combine(state, Aspen::State::CONTINUE);
      }
    } else {
      ++completionCount;
    }
  }
}

#endif
