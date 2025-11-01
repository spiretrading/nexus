#ifndef NEXUS_RISK_PARAMETERS_HPP
#define NEXUS_RISK_PARAMETERS_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {

  /** Stores the list of parameters used to measure an account's risk. */
  struct RiskParameters {

    /** The currency used for risk calculations. */
    CurrencyId m_currency;

    /** The maximum amount of buying power. */
    Money m_buying_power;

    /** The state that the account is allowed to be in. */
    RiskState m_allowed_state;

    /** The max net loss before entering closed orders mode. */
    Money m_net_loss;

    /**
     * The amount of time allowed to transition from closed orders mode to
     * disabled mode.
     */
    boost::posix_time::time_duration m_transition_time;

    bool operator ==(const RiskParameters&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const RiskParameters& parameters) {
    return out << '(' << parameters.m_currency << ' ' <<
      parameters.m_buying_power << ' ' << parameters.m_allowed_state << ' ' <<
      parameters.m_net_loss << ' ' << parameters.m_transition_time << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::RiskParameters> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::RiskParameters& value,
        unsigned int version) const {
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("buying_power", value.m_buying_power);
      shuttle.shuttle("allowed_state", value.m_allowed_state);
      shuttle.shuttle("net_loss", value.m_net_loss);
      shuttle.shuttle("transition_time", value.m_transition_time);
    }
  };
}

#endif
