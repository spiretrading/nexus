#ifndef NEXUS_DEFINITIONS_LISTING_HPP
#define NEXUS_DEFINITIONS_LISTING_HPP
#include <cstdint>
#include <functional>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Instrument.hpp"

namespace Nexus {

  /** Represents a listing. */
  struct Listing {

    /** The listing id. */
    std::uint64_t m_id;

    /** The listed instrument. */
    Instrument m_instrument;

    constexpr bool operator ==(const Listing& rhs) const;
  };
}

constexpr bool Nexus::Listing::operator ==(const Listing& rhs) const {
  return m_id == rhs.m_id;
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Listing> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::Listing& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("instrument", value.m_instrument);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Listing> {
    std::size_t operator ()(const Nexus::Listing& value) const noexcept {
      return std::hash<std::uint64_t>()(value.m_id);
    }
  };
}

#endif
