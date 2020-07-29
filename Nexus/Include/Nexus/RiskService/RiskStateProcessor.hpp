#ifndef NEXUS_RISK_STATE_PROCESSOR_HPP
#define NEXUS_RISK_STATE_PROCESSOR_HPP
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::RiskService {

  /**
   * Monitors a Portfolio against a set of RiskParameters.
   * @param <P> The type of Portfolio to monitor.
   * @param <T> The type of TimeClient used.
   */
  template<typename P, typename T>
  class RiskStateProcessor {
    public:

      /** The type of Portfolio to monitor. */
      using Portfolio = P;

      /** The type of TimeClient used. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a RiskStateProcessor.
       * @param portfolio The initial state of the Portfolio.
       * @param parameters The initial RiskParameters.
       * @param exchangeRates The list of ExchangeRates.
       * @param timeClient Initializes the TimeClient.
       */
      template<typename TF>
      RiskStateProcessor(Portfolio portfolio, RiskParameters parameters,
        const std::vector<ExchangeRate>& exchangeRates, TF&& timeClient);

      /** Returns the RiskParameters. */
      const RiskParameters& GetParameters() const;

      /** Returns the Portfolio. */
      const Portfolio& GetPortfolio() const;

      /** Returns the Portfolio. */
      Portfolio& GetPortfolio();

      /** Returns the current RiskState. */
      const RiskState& GetRiskState() const;

      /**
       * Updates the RiskParameters.
       * @param parameters The new RiskParameters to use.
       */
      void Update(const RiskParameters& parameters);

      /**
       * Updates the transition timer.
       * @param period The amount of time elapsed since the last timer expiry.
       */
      void Update(boost::posix_time::time_duration period);

      /** Updates based on a change to the Portfolio. */
      void Update();

    private:
      Portfolio m_portfolio;
      RiskParameters m_parameters;
      ExchangeRateTable m_exchangeRates;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      RiskState m_riskState;
      boost::posix_time::time_duration m_transitionTimeRemaining;
  };

  template<typename Portfolio, typename TimeClient>
  RiskStateProcessor(Portfolio, RiskParameters,
    const std::vector<ExchangeRate>&, TimeClient&&) ->
    RiskStateProcessor<Portfolio, std::decay_t<TimeClient>>;

  template<typename P, typename T>
  template<typename TF>
  RiskStateProcessor<P, T>::RiskStateProcessor(Portfolio portfolio,
      RiskParameters parameters, const std::vector<ExchangeRate>& exchangeRates,
      TF&& timeClient)
      : m_portfolio(std::move(portfolio)),
        m_parameters(std::move(parameters)),
        m_timeClient(std::forward<TF>(timeClient)) {
    for(auto& exchangeRate : exchangeRates) {
      m_exchangeRates.Add(exchangeRate);
    }
  }

  template<typename P, typename T>
  const RiskParameters& RiskStateProcessor<P, T>::GetParameters() const {
    return m_parameters;
  }

  template<typename P, typename T>
  const typename RiskStateProcessor<P, T>::Portfolio&
      RiskStateProcessor<P, T>::GetPortfolio() const {
    return m_portfolio;
  }

  template<typename P, typename T>
  typename RiskStateProcessor<P, T>::Portfolio&
      RiskStateProcessor<P, T>::GetPortfolio() {
    return m_portfolio;
  }

  template<typename P, typename T>
  const RiskState& RiskStateProcessor<P, T>::GetRiskState() const {
    return m_riskState;
  }

  template<typename P, typename T>
  void RiskStateProcessor<P, T>::Update(const RiskParameters& parameters) {
    m_parameters = parameters;
    if(m_riskState.m_type == RiskState::Type::ACTIVE) {
      m_riskState = m_parameters.m_allowedState;
    }
    Update();
  }

  template<typename P, typename T>
  void RiskStateProcessor<P, T>::Update(
      boost::posix_time::time_duration period) {
    if(m_riskState.m_type == RiskState::Type::CLOSE_ORDERS) {
      m_transitionTimeRemaining -= period;
      if(m_transitionTimeRemaining <= boost::posix_time::seconds(0)) {
        m_riskState = RiskState::Type::DISABLED;
      }
    }
  }

  template<typename P, typename T>
  void RiskStateProcessor<P, T>::Update() {
    if(m_parameters.m_currency == CurrencyId::NONE) {
      return;
    }
    auto profitAndLoss = Money::ZERO;
    for(auto currency : m_portfolio.GetUnrealizedProfitAndLosses() |
        boost::adaptors::map_keys) {
      try {
        auto currencyProfitAndLoss =
          Accounting::GetTotalProfitAndLoss(m_portfolio, currency);
        auto convertedProfitAndLoss = m_exchangeRates.Convert(
          currencyProfitAndLoss, currency, m_parameters.m_currency);
        profitAndLoss += convertedProfitAndLoss;
      } catch(const CurrencyPairNotFoundException&) {
        std::cerr << "Currency pair not found: " << currency << " " <<
          m_parameters.m_currency << std::endl;
        if(m_riskState.m_type == RiskState::Type::ACTIVE) {
          m_riskState.m_type = RiskState::Type::CLOSE_ORDERS;
          m_riskState.m_expiry = m_timeClient->GetTime() +
            m_transitionTimeRemaining;
        }
        return;
      }
    }
    if(m_riskState.m_type == RiskState::Type::ACTIVE) {
      if(profitAndLoss <= -m_parameters.m_netLoss) {
        m_transitionTimeRemaining = m_parameters.m_transitionTime;
        m_riskState.m_type = RiskState::Type::CLOSE_ORDERS;
        m_riskState.m_expiry = m_timeClient->GetTime() +
          m_transitionTimeRemaining;
      }
    } else {
      if(profitAndLoss > -m_parameters.m_netLoss) {
        m_riskState = m_parameters.m_allowedState;
      }
    }
  }
}

#endif
