#ifndef NEXUS_RISK_PARAMETERS_HPP
#define NEXUS_RISK_PARAMETERS_HPP
#include <ostream>
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

    /** Constructs default RiskParameters. */
    RiskParameters();

    /**
     * Constructs RiskParameters.
     * @param currency The currency used for risk calculations.
     * @param buyingPower The maximum amount of buying power.
     * @param allowedState The state that the account is allowed to be in.
     * @param netLoss The max net loss before entering closed orders mode.
     * @param lossFromTop The percentage lost from the top before entering
     *        closed orders mode.
     * @param transitionTime The amount of time allowed to transition from
     *        closed orders mode to disabled mode.
     */
    RiskParameters(CurrencyId currency, Money buyingPower,
      RiskState allowedState, Money netLoss, int lossFromTop,
      boost::posix_time::time_duration transitionTime);

    bool operator ==(const RiskParameters& riskParameters) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const RiskParameters& parameters) {
    return out << '(' << parameters.m_currency << ' ' <<
      parameters.m_buyingPower << ' ' << parameters.m_allowedState << ' ' <<
      parameters.m_netLoss << ' ' << parameters.m_lossFromTop << ' ' <<
      parameters.m_transitionTime << ')';
  }

  inline RiskParameters::RiskParameters()
    : m_lossFromTop(0) {}

  inline RiskParameters::RiskParameters(CurrencyId currency, Money buyingPower,
    RiskState allowedState, Money netLoss, int lossFromTop,
    boost::posix_time::time_duration transitionTime)
    : m_currency(currency),
      m_buyingPower(buyingPower),
      m_allowedState(allowedState),
      m_netLoss(netLoss),
      m_lossFromTop(lossFromTop),
      m_transitionTime(transitionTime) {}
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
