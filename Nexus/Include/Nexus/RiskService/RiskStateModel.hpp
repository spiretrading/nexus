#ifndef NEXUS_RISK_STATE_MODEL_HPP
#define NEXUS_RISK_STATE_MODEL_HPP
#include <iostream>
#include <ranges>
#include <utility>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus {

  /**
   * Monitors a Portfolio against a set of RiskParameters.
   * @param <T> The type of TimeClient used.
   */
  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  class RiskStateModel {
    public:

      /** The type of TimeClient used. */
      using TimeClient = Beam::dereference_t<T>;

      /**
       * Constructs a RiskStateModel.
       * @param portfolio The initial state of the Portfolio.
       * @param parameters The initial RiskParameters.
       * @param exchange_rates The exchange rates used to calculate loss.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<T> TF>
      RiskStateModel(RiskPortfolio portfolio, const RiskParameters& parameters,
        const ExchangeRateTable& exchange_rates, TF&& time_client);

      /** Returns the RiskParameters. */
      const RiskParameters& get_parameters() const;

      /** Returns the Portfolio. */
      const RiskPortfolio& get_portfolio() const;

      /** Returns the Portfolio. */
      RiskPortfolio& get_portfolio();

      /** Returns the current RiskState. */
      const RiskState& get_risk_state() const;

      /**
       * Updates the RiskParameters.
       * @param parameters The new RiskParameters to use.
       */
      void update(const RiskParameters& parameters);

      /** Updates based on the time. */
      void update_time();

      /** Updates based on a change to the Portfolio. */
      void update_portfolio();

    private:
      RiskPortfolio m_portfolio;
      RiskParameters m_parameters;
      ExchangeRateTable m_exchange_rates;
      Beam::local_ptr_t<T> m_time_client;
      RiskState m_risk_state;
  };

  template<typename C>
  RiskStateModel(RiskPortfolio, const RiskParameters&, const ExchangeRateTable&,
    C&&) -> RiskStateModel<std::remove_cvref_t<C>>;

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  RiskStateModel<T>::RiskStateModel(RiskPortfolio portfolio,
      const RiskParameters& parameters,
      const ExchangeRateTable& exchange_rates, TF&& time_client)
    : m_portfolio(std::move(portfolio)),
      m_exchange_rates(exchange_rates),
      m_time_client(std::forward<TF>(time_client)),
      m_risk_state(parameters.m_allowed_state) {
    update(parameters);
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  const RiskParameters& RiskStateModel<T>::get_parameters() const {
    return m_parameters;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  const RiskPortfolio& RiskStateModel<T>::get_portfolio() const {
    return m_portfolio;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  RiskPortfolio& RiskStateModel<T>::get_portfolio() {
    return m_portfolio;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  const RiskState& RiskStateModel<T>::get_risk_state() const {
    return m_risk_state;
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void RiskStateModel<T>::update(const RiskParameters& parameters) {
    m_parameters = parameters;
    if(m_risk_state.m_type == RiskState::Type::ACTIVE) {
      m_risk_state = m_parameters.m_allowed_state;
    }
    update_time();
    update_portfolio();
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void RiskStateModel<T>::update_time() {
    if(m_risk_state.m_type == RiskState::Type::CLOSE_ORDERS &&
        m_time_client->get_time() >= m_risk_state.m_expiry) {
      m_risk_state = RiskState::Type::DISABLED;
    }
  }

  template<typename T> requires Beam::IsTimeClient<Beam::dereference_t<T>>
  void RiskStateModel<T>::update_portfolio() {
    if(!m_parameters.m_currency) {
      return;
    }
    auto profit_and_loss = Money::ZERO;
    for(auto currency :
        m_portfolio.get_unrealized_profit_and_losses() | std::views::keys) {
      try {
        auto currency_profit_and_loss =
          get_total_profit_and_loss(m_portfolio, currency);
        auto converted_profit_and_loss = m_exchange_rates.convert(
          currency_profit_and_loss, currency, m_parameters.m_currency);
        profit_and_loss += converted_profit_and_loss;
      } catch(const CurrencyPairNotFoundException&) {
        std::cerr << "Currency pair not found: " << currency << ' ' <<
          m_parameters.m_currency << std::endl;
        if(m_risk_state.m_type == RiskState::Type::ACTIVE) {
          m_risk_state.m_type = RiskState::Type::CLOSE_ORDERS;
          m_risk_state.m_expiry =
            m_time_client->get_time() + m_parameters.m_transition_time;
        }
        return;
      }
    }
    if(m_risk_state.m_type == RiskState::Type::ACTIVE) {
      if(profit_and_loss <= -m_parameters.m_net_loss) {
        m_risk_state.m_type = RiskState::Type::CLOSE_ORDERS;
        m_risk_state.m_expiry =
          m_time_client->get_time() + m_parameters.m_transition_time;
      }
    } else {
      if(profit_and_loss > -m_parameters.m_net_loss) {
        m_risk_state = m_parameters.m_allowed_state;
      }
    }
  }
}

#endif
