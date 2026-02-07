#ifndef NEXUS_DEFINITIONS_INSTRUMENT_HPP
#define NEXUS_DEFINITIONS_INSTRUMENT_HPP
#include <cstdint>
#include <functional>
#include <ios>
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Asset.hpp"

namespace Nexus {

  /** Represents a financial instrument. */
  struct Instrument {

    /** Enumerates supported instrument types. */
    enum class Type : std::uint8_t {

      /** No instrument type. */
      NONE = 0,

      /** The spot market. */
      SPOT,

      /** A forward contract. */
      FORWARD,

      /** A futures contract. */
      FUTURE,

      /** A perpetual contract. */
      PERPETUAL,

      /** An options contract. */
      OPTION,

      /** A swap contract. */
      SWAP,

      /** A contract for difference. */
      CFD
    };

    /** The base asset. */
    Asset m_base;

    /** The quote asset. */
    Asset m_quote;

    /** The instrument type. */
    Type m_type;

    bool operator ==(const Instrument&) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out, Instrument::Type type) {
    if(type == Instrument::Type::NONE) {
      return out << "NONE";
    } else if(type == Instrument::Type::SPOT) {
      return out << "SPT";
    } else if(type == Instrument::Type::FORWARD) {
      return out << "FWD";
    } else if(type == Instrument::Type::FUTURE) {
      return out << "FUT";
    } else if(type == Instrument::Type::PERPETUAL) {
      return out << "PERP";
    } else if(type == Instrument::Type::OPTION) {
      return out << "OPT";
    } else if(type == Instrument::Type::SWAP) {
      return out << "SWP";
    } else if(type == Instrument::Type::CFD) {
      return out << "CFD";
    }
    out.setstate(std::ios::failbit);
    return out;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Instrument> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Instrument& value, unsigned int version) const {
      shuttle.shuttle("base", value.m_base);
      shuttle.shuttle("quote", value.m_quote);
      shuttle.shuttle("type", value.m_type);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Instrument> {
    std::size_t operator ()(const Nexus::Instrument& value) const noexcept {
      auto seed = std::hash<Nexus::Asset>()(value.m_base);
      auto quote_hash = std::hash<Nexus::Asset>()(value.m_quote);
      seed ^= quote_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      auto type_hash = std::hash<int>()(static_cast<int>(value.m_type));
      seed ^= type_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };
}

#endif
