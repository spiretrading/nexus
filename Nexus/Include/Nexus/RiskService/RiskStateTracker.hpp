#ifndef NEXUS_RISK_STATE_TRACKER_HPP
#define NEXUS_RISK_STATE_TRACKER_HPP
#include <iostream>
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
  class RiskStateTracker {
    public:

      /** The type of Portfolio to monitor. */
      using Portfolio = P;

      /** The type of TimeClient used. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a RiskStateTracker.
       * @param portfolio The initial state of the Portfolio.
       * @param exchangeRates The list of ExchangeRates.
       * @param timeClient Initializes the TimeClient.
       */
      template<typename TF>
      RiskStateTracker(const Portfolio& portfolio,
        const std::vector<ExchangeRate>& exchangeRates, TF&& timeClient);

      /** Returns the RiskParameters. */
      const RiskParameters& GetRiskParameters() const;

      /** Returns the Portfolio. */
      const Portfolio& GetPortfolio() const;

      /** Returns the Portfolio. */
      Portfolio& GetPortfolio();

      /** Returns the current RiskState. */
      const RiskState& GetRiskState() const;

      /**
       * Updates the RiskParameters.
       * @param riskParameters The new RiskParameters to use.
       */
      void Update(const RiskParameters& riskParameters);

      /**
       * Updates the transition timer.
       * @param period The amount of time elapsed since the last timer expiry.
       */
      void Update(boost::posix_time::time_duration period);

      /** Updates based on a change to the Portfolio. */
      void Update();

    private:
      RiskParameters m_riskParameters;
      Portfolio m_portfolio;
      ExchangeRateTable m_exchangeRates;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      RiskState m_riskState;
      boost::posix_time::time_duration m_transitionTimeRemaining;
  };

  template<typename P, typename T>
  template<typename TF>
  RiskStateTracker<P, T>::RiskStateTracker(const Portfolio& portfolio,
      const std::vector<ExchangeRate>& exchangeRates, TF&& timeClient)
      : m_portfolio(portfolio),
        m_timeClient(std::forward<TF>(timeClient)) {
    for(auto& exchangeRate : exchangeRates) {
      m_exchangeRates.Add(exchangeRate);
    }
  }

  template<typename P, typename T>
  const RiskParameters& RiskStateTracker<P, T>::GetRiskParameters() const {
    return m_riskParameters;
  }

  template<typename P, typename T>
  const typename RiskStateTracker<P, T>::Portfolio&
      RiskStateTracker<P, T>::GetPortfolio() const {
    return m_portfolio;
  }

  template<typename P, typename T>
  typename RiskStateTracker<P, T>::Portfolio&
      RiskStateTracker<P, T>::GetPortfolio() {
    return m_portfolio;
  }

  template<typename P, typename T>
  const RiskState& RiskStateTracker<P, T>::GetRiskState() const {
    return m_riskState;
  }

  template<typename P, typename T>
  void RiskStateTracker<P, T>::Update(const RiskParameters& riskParameters) {
    m_riskParameters = riskParameters;
    if(m_riskState.m_type == RiskState::Type::ACTIVE) {
      m_riskState = m_riskParameters.m_allowedState;
    }
    Update();
  }

  template<typename P, typename T>
  void RiskStateTracker<P, T>::Update(boost::posix_time::time_duration period) {
    if(m_riskState.m_type == RiskState::Type::CLOSE_ORDERS) {
      m_transitionTimeRemaining -= period;
      if(m_transitionTimeRemaining <= boost::posix_time::seconds(0)) {
        m_riskState = RiskState::Type::DISABLED;
      }
    }
  }

  template<typename P, typename T>
  void RiskStateTracker<P, T>::Update() {
    if(m_riskParameters.m_currency == CurrencyId::NONE) {
      return;
    }
    auto profitAndLoss = Money::ZERO;
    for(auto currency : m_portfolio.GetUnrealizedProfitAndLosses() |
        boost::adaptors::map_keys) {
      try {
        auto currencyProfitAndLoss =
          Accounting::GetTotalProfitAndLoss(m_portfolio, currency);
        auto convertedProfitAndLoss = m_exchangeRates.Convert(
          currencyProfitAndLoss, currency, m_riskParameters.m_currency);
        profitAndLoss += convertedProfitAndLoss;
      } catch(const std::exception&) {
        std::cerr << "Currency pair not found: " << currency << " " <<
          m_riskParameters.m_currency << std::endl;
        if(m_riskState.m_type == RiskState::Type::ACTIVE) {
          m_riskState.m_type = RiskState::Type::CLOSE_ORDERS;
          m_riskState.m_expiry = m_timeClient->GetTime() +
            m_transitionTimeRemaining;
        }
        return;
      }
    }
    if(m_riskState.m_type == RiskState::Type::ACTIVE) {
      if(profitAndLoss <= -m_riskParameters.m_netLoss) {
        m_transitionTimeRemaining = m_riskParameters.m_transitionTime;
        m_riskState.m_type = RiskState::Type::CLOSE_ORDERS;
        m_riskState.m_expiry = m_timeClient->GetTime() +
          m_transitionTimeRemaining;
      }
    } else {
      if(profitAndLoss > -m_riskParameters.m_netLoss) {
        m_riskState = m_riskParameters.m_allowedState;
      }
    }
  }
}

#endif
