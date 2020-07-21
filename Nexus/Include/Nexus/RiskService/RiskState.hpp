#ifndef NEXUS_RISK_STATE_HPP
#define NEXUS_RISK_STATE_HPP
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
     * Constructs a RiskState.
     * @param type The state's type.
     * @param expiry When this state is expected to expire.
     */
    RiskState(Type type, boost::posix_time::ptime expiry);

    /**
     * Assigns a RiskState with an indefinite expiry.
     * @param type The Type to assign to <code>this</code>.
     * @return A reference to <code>this</code>.
     */
    RiskState& operator =(RiskState::Type type);

    /**
     * Tests a RiskState for equality.
     * @param rhs The right hand side of the equality.
     * @return <code>true</code> if the Types and expiries are equal.
     */
    bool operator ==(const RiskState& rhs) const;

    /**
     * Tests a RiskState for inequality.
     * @param rhs The right hand side of the equality.
     * @return <code>true</code> if the Types or expiries are not equal.
     */
    bool operator !=(const RiskState& rhs) const;
  };

  inline RiskState::RiskState()
    : m_type(Type::ACTIVE),
      m_expiry(boost::posix_time::pos_infin) {}

  inline RiskState::RiskState(Type type)
    : m_type(type),
      m_expiry(boost::posix_time::pos_infin) {}

  inline RiskState::RiskState(Type type, boost::posix_time::ptime expiry)
    : m_type(type),
      m_expiry(expiry) {}

  inline RiskState& RiskState::operator =(RiskState::Type type) {
    m_type = type;
    m_expiry = boost::posix_time::pos_infin;
    return *this;
  }

  inline bool RiskState::operator ==(const RiskState& rhs) const {
    return m_type == rhs.m_type && m_expiry == rhs.m_expiry;
  }

  inline bool RiskState::operator !=(const RiskState& rhs) const {
    return !(*this == rhs);
  }
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
