#ifndef NEXUS_RISK_PARAMETERS_HPP
#define NEXUS_RISK_PARAMETERS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::RiskService {

  /** Stores the list of parameters used to measure an account's risk. */
  struct RiskParameters {

    /** Constructs default RiskParameters. */
    RiskParameters();

    /** The currency used for risk calculations. */
    CurrencyId m_currency;

    /** The maximum amount of buying power. */
    Money m_buyingPower;

    /** The state that the account is allowed to be in. */
    RiskState m_allowedState;

    /** The max net loss before entering closed orders mode. */
    Money m_netLoss;

    /** The percentage lost from the top before entering closed orders mode. */
    int m_lossFromTop;

    /**
     * The amount of time allowed to transition from closed orders mode to
     * disabled mode.
     */
    boost::posix_time::time_duration m_transitionTime;

    /** Tests two RiskParameters for equality. */
    bool operator ==(const RiskParameters& riskParameters) const;

    /** Tests two RiskParameters for inequality. */
    bool operator !=(const RiskParameters& riskParameters) const;
  };

  inline RiskParameters::RiskParameters()
    : m_lossFromTop(0) {}

  inline bool RiskParameters::operator ==(
      const RiskParameters& riskParameters) const {
    return m_currency == riskParameters.m_currency &&
      m_buyingPower == riskParameters.m_buyingPower && m_allowedState ==
      riskParameters.m_allowedState && m_netLoss == riskParameters.m_netLoss &&
      m_lossFromTop == riskParameters.m_lossFromTop && m_transitionTime ==
      riskParameters.m_transitionTime;
  }

  inline bool RiskParameters::operator !=(
      const RiskParameters& riskParameters) const {
    return !(*this == riskParameters);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::RiskParameters> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::RiskService::RiskParameters& value, unsigned int version) {
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("buying_power", value.m_buyingPower);
      shuttle.Shuttle("allowed_state", value.m_allowedState);
      shuttle.Shuttle("net_loss", value.m_netLoss);
      shuttle.Shuttle("loss_from_top", value.m_lossFromTop);
      shuttle.Shuttle("transition_time", value.m_transitionTime);
    }
  };
}

#endif
