#ifndef NEXUS_SECURITY_INFO_HPP
#define NEXUS_SECURITY_INFO_HPP
#include <string>
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Definitions.hpp"
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
    Quantity m_boardLot;

    bool operator ==(const SecurityInfo& rhs) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const SecurityInfo& value) {
    return out << '(' << value.m_security << ' ' << value.m_name << ' ' <<
      value.m_sector << ' ' << value.m_boardLot << ')';
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::SecurityInfo> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::SecurityInfo& value,
        unsigned int version) {
      shuttle.Shuttle("security", value.m_security);
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("sector", value.m_sector);
      shuttle.Shuttle("board_lot", value.m_boardLot);
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
