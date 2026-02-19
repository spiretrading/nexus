#ifndef NEXUS_DEFINITIONS_TIME_IN_FORCE_HPP
#define NEXUS_DEFINITIONS_TIME_IN_FORCE_HPP
#include <functional>
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Utilities/HashPosixTimeTypes.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/functional/hash.hpp>

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
      TimeInForce(Type type) noexcept;

      /**
       * Constructs a TimeInForce.
       * @param type The TimeInForce's Type.
       */
      TimeInForce(Type::Type type) noexcept;

      /**
       * Constructs a TimeInForce.
       * @param type The TimeInForce's Type.
       * @param expiry Specifies when the Order expires.
       */
      TimeInForce(Type type, boost::posix_time::ptime expiry) noexcept;

      /** Returns the Type. */
      Type get_type() const;

      /** Returns the expiry. */
      boost::posix_time::ptime get_expiry() const;

      bool operator ==(const TimeInForce&) const = default;

    private:
      friend struct Beam::Shuttle<TimeInForce>;
      Type m_type;
      boost::posix_time::ptime m_expiry;
  };

  /**
   * Makes a TimeInForce whose type is GTD.
   * @param expiry Specifies when the Order expires.
   */
  inline TimeInForce make_gtd(boost::posix_time::ptime expiry) {
    return TimeInForce(TimeInForce::Type::GTD, expiry);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const TimeInForce& value) {
    if(value.get_expiry() == boost::posix_time::not_a_date_time) {
      return out << value.get_type();
    }
    return out << '(' << value.get_type() << ' ' << value.get_expiry() << ')';
  }

  inline TimeInForce::TimeInForce(Type type) noexcept
    : m_type(type) {}

  inline TimeInForce::TimeInForce(Type::Type type) noexcept
    : TimeInForce(Type(type)) {}

  inline TimeInForce::TimeInForce(
    Type type, boost::posix_time::ptime expiry) noexcept
    : m_type(type),
      m_expiry(expiry) {}

  inline TimeInForce::Type TimeInForce::get_type() const {
    return m_type;
  }

  inline boost::posix_time::ptime TimeInForce::get_expiry() const {
    return m_expiry;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TimeInForce> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::TimeInForce& value, unsigned int version) const {
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("expiry", value.m_expiry);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::TimeInForce> {
    std::size_t operator ()(
        const Nexus::TimeInForce& value) const noexcept {
      auto seed = std::size_t(0);
      boost::hash_combine(seed,
        std::hash<Nexus::TimeInForce::Type>()(value.get_type()));
      boost::hash_combine(seed,
        std::hash<boost::posix_time::ptime>()(value.get_expiry()));
      return seed;
    }
  };
}

#endif
