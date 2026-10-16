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

template<>
class std::formatter<Nexus::RiskParameters> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::RiskParameters& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {} {})", value.m_currency,
          value.m_buying_power, value.m_allowed_state, value.m_net_loss,
          Beam::to_string(value.m_transition_time)), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, const RiskParameters& value) {
    auto database = static_cast<const CurrencyDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<CurrencyDatabase>::ID));
    if(database) {
      return out <<
        std::format(make_locale(out.getloc(), *database), "{}", value);
    }
    return out << std::format(out.getloc(), "{}", value);
  }
}

#endif
