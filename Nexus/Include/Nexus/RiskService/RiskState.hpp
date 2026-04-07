#ifndef NEXUS_RISK_STATE_HPP
#define NEXUS_RISK_STATE_HPP
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace Nexus {
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
    RiskState() noexcept;

    /**
     * Constructs a RiskState with an indefinite expiry.
     * @param type The state's type.
     */
    RiskState(Type type) noexcept;

    /**
     * Constructs a RiskState with an indefinite expiry.
     * @param type The state's type.
     */
    RiskState(Type::Type type) noexcept;

    /**
     * Constructs a RiskState.
     * @param type The state's type.
     * @param expiry When this state is expected to expire.
     */
    RiskState(Type type, boost::posix_time::ptime expiry) noexcept;

    bool operator ==(const RiskState&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, RiskState::Type type) {
    return out << static_cast<RiskState::Type::Type>(type);
  }

  inline std::ostream& operator <<(std::ostream& out, const RiskState& state) {
    return out << '(' << state.m_type << ' ' << state.m_expiry << ')';
  }

  inline RiskState::RiskState() noexcept
    : RiskState(Type::ACTIVE) {}

  inline RiskState::RiskState(Type type) noexcept
    : RiskState(type, boost::posix_time::pos_infin) {}

  inline RiskState::RiskState(Type::Type type) noexcept
    : RiskState(Type(type)) {}

  inline RiskState::RiskState(
    Type type, boost::posix_time::ptime expiry) noexcept
    : m_type(type),
      m_expiry(expiry) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::RiskState> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::RiskState& value, unsigned int version) const {
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("expiry", value.m_expiry);
    }
  };
}

#endif
