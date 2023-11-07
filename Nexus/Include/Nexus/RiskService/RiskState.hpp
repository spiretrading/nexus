#ifndef NEXUS_RISK_STATE_HPP
#define NEXUS_RISK_STATE_HPP
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {
namespace Details {
  BEAM_ENUM(RiskStateTypeDefinition,

    /** The account is allowed to submit Orders. */
    ACTIVE,

    /** The account may only submit Orders to close a position. */
    CLOSE_ORDERS,

    /** The account may no longer submit Orders. */
    DISABLED);
}

  /** Stores the risk monitoring state of an account. */
  struct RiskState {

    /**
     * The state that an account can be in for the purpose of risk monitoring.
     */
    using Type = Details::RiskStateTypeDefinition;

    /** The state's type. */
    Type m_type;

    /** When this state is expected to expire. */
    boost::posix_time::ptime m_expiry;

    /** Constructs an uninitialized RiskState. */
    RiskState();

    /**
     * Constructs a RiskState with an indefinite expiry.
     * @param type The state's type.
     */
    RiskState(Type type);

    /**
     * Constructs a RiskState with an indefinite expiry.
     * @param type The state's type.
     */
    RiskState(Type::Type type);

    /**
     * Constructs a RiskState.
     * @param type The state's type.
     * @param expiry When this state is expected to expire.
     */
    RiskState(Type type, boost::posix_time::ptime expiry);

    bool operator ==(const RiskState& rhs) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out, RiskState::Type type) {
    if(type == RiskState::Type::ACTIVE) {
      return out << "ACTIVE";
    } else if(type == RiskState::Type::CLOSE_ORDERS) {
      return out << "CLOSE_ORDERS";
    } else if(type == RiskState::Type::DISABLED) {
      return out << "DISABLED";
    } else {
      return out << "NONE";
    }
  }

  inline std::ostream& operator <<(std::ostream& out, const RiskState& state) {
    return out << '(' << state.m_type << ' ' << state.m_expiry << ')';
  }

  inline RiskState::RiskState()
    : RiskState(Type::ACTIVE) {}

  inline RiskState::RiskState(Type type)
    : RiskState(type, boost::posix_time::pos_infin) {}

  inline RiskState::RiskState(Type::Type type)
    : RiskState(Type(type)) {}

  inline RiskState::RiskState(Type type, boost::posix_time::ptime expiry)
    : m_type(type),
      m_expiry(expiry) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::RiskState> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::RiskService::RiskState& value,
        unsigned int version) {
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("expiry", value.m_expiry);
    }
  };
}

#endif
