#ifndef NEXUS_TIME_IN_FORCE_HPP
#define NEXUS_TIME_IN_FORCE_HPP
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {
namespace Details {
  BEAM_ENUM(TimeInForceTypeDefinition,

    /** Day (or session). */
    DAY,

    /** Good until cancelled. */
    GTC,

    /** At the opening. */
    OPG,

    /** At the close. */
    MOC,

    /** Immediate or cancel. */
    IOC,

    /** Fill or kill. */
    FOK,

    /** Good until crossing. */
    GTX,

    /** Good until date. */
    GTD);
}

  /** Represents an instruction used to specify when an Order expires. */
  class TimeInForce {
    public:

      /** Lists the TimeInForce types. */
      using Type = Details::TimeInForceTypeDefinition;

      /** Constructs an uninitialized TimeInForce. */
      TimeInForce() = default;

      /**
       * Constructs a TimeInForce.
       * @param type The TimeInForce's Type.
       */
      TimeInForce(Type type);

      /**
       * Constructs a TimeInForce.
       * @param type The TimeInForce's Type.
       * @param expiry Specifies when the Order expires.
       */
      TimeInForce(Type type, boost::posix_time::ptime expiry);

      /** Returns the Type. */
      Type GetType() const;

      /** Returns the expiry. */
      boost::posix_time::ptime GetExpiry() const;

      bool operator ==(const TimeInForce& timeInForce) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<TimeInForce>;
      Type m_type;
      boost::posix_time::ptime m_expiry;
  };

  inline std::ostream& operator <<(std::ostream& out,
      TimeInForce::Type value) {
    if(value == Nexus::TimeInForce::Type::DAY) {
      return out << "DAY";
    } else if(value == Nexus::TimeInForce::Type::GTC) {
      return out << "GTC";
    } else if(value == Nexus::TimeInForce::Type::OPG) {
      return out << "OPG";
    } else if(value == Nexus::TimeInForce::Type::MOC) {
      return out << "MOC";
    } else if(value == Nexus::TimeInForce::Type::IOC) {
      return out << "IOC";
    } else if(value == Nexus::TimeInForce::Type::FOK) {
      return out << "FOK";
    } else if(value == Nexus::TimeInForce::Type::GTX) {
      return out << "GTX";
    } else if(value == Nexus::TimeInForce::Type::GTD) {
      return out << "GTD";
    }
    return out << "NONE";
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TimeInForce& value) {
    if(value.GetExpiry() == boost::posix_time::not_a_date_time) {
      return out << value.GetType();
    } else {
      return out << "(" << value.GetType() << " " << value.GetExpiry() << ")";
    }
  }

  inline TimeInForce::TimeInForce(Type type)
    : m_type(type) {}

  inline TimeInForce::TimeInForce(Type type, boost::posix_time::ptime expiry)
    : m_type(type),
      m_expiry(expiry) {}

  inline TimeInForce::Type TimeInForce::GetType() const {
    return m_type;
  }

  inline boost::posix_time::ptime TimeInForce::GetExpiry() const {
    return m_expiry;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::TimeInForce> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TimeInForce& value,
        unsigned int version) {
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("expiry", value.m_expiry);
    }
  };
}

#endif
