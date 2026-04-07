#ifndef NEXUS_SECURITY_INFO_HPP
#define NEXUS_SECURITY_INFO_HPP
#include <functional>
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Security.hpp"

namespace Nexus {

  /** Stores info about the company/entity represented by a Security. */
  struct SecurityInfo {

    /** The Security represented. */
    Security m_security;

    /** The full name of the entity. */
    std::string m_name;

    /** The sector the Security belongs to. */
    std::string m_sector;

    /** The board lot. */
    Quantity m_board_lot;

    bool operator ==(const SecurityInfo&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const SecurityInfo& value) {
    return out << '(' << value.m_security << ' ' << value.m_name << ' ' <<
      value.m_sector << ' ' << value.m_board_lot << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::SecurityInfo> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::SecurityInfo& value, unsigned int version) const {
      shuttle.shuttle("security", value.m_security);
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("sector", value.m_sector);
      shuttle.shuttle("board_lot", value.m_board_lot);
    }
  };
}

namespace std {
  template <>
  struct hash<Nexus::SecurityInfo> {
    size_t operator()(const Nexus::SecurityInfo& value) const {
      return Nexus::hash_value(value.m_security);
    }
  };
}

#endif
