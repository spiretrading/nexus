#ifndef NEXUS_SECURITY_INFO_HPP
#define NEXUS_SECURITY_INFO_HPP
#include <string>
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Security.hpp"

namespace Nexus {

  /*! \struct SecurityInfo
      \brief Stores info about the company/entity represented by a Security.
   */
  struct SecurityInfo {

    //! The Security represented.
    Security m_security;

    //! The full name of the entity.
    std::string m_name;

    //! The sector the Security belongs to.
    std::string m_sector;

    //! The board lot.
    Quantity m_boardLot;

    //! Constructs an empty SecurityInfo.
    SecurityInfo() = default;

    //! Constructs a SecurityInfo.
    /*!
      \param security The Security represented.
      \param name The full name of the entity.
      \param sector The sector the Security belongs to.
      \param boardLot The board lot.
    */
    SecurityInfo(Security security, std::string name, std::string sector,
      Quantity boardLot);

    //! Tests whether this SecurityInfo is structurally equal to another.
    /*!
      \param rhs The right hand side of the equality.
      \return <code>true</code> iff <i>this</i> is structurally equal to
              <i>rhs</i>.
    */
    bool operator ==(const SecurityInfo& rhs) const;

    //! Tests whether this SecurityInfo is not structurally equal to another.
    /*!
      \param rhs The right hand side of the equality.
      \return <code>true</code> iff <i>this</i> is not structurally equal to
              <i>rhs</i>.
    */
    bool operator !=(const SecurityInfo& rhs) const;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const SecurityInfo& value) {
    return out << '(' << value.m_security << ' ' << value.m_name << ' ' <<
      value.m_sector << ' ' << value.m_boardLot << ')';
  }

  inline SecurityInfo::SecurityInfo(Security security, std::string name,
    std::string sector, Quantity boardLot)
    : m_security(std::move(security)),
      m_name(std::move(name)),
      m_sector(std::move(sector)),
      m_boardLot(std::move(boardLot)) {}

  inline bool SecurityInfo::operator ==(const SecurityInfo& rhs) const {
    return std::tie(m_security, m_name, m_sector, m_boardLot) ==
      std::tie(rhs.m_security, rhs.m_name, rhs.m_sector, rhs.m_boardLot);
  }

  inline bool SecurityInfo::operator !=(const SecurityInfo& rhs) const {
    return !(*this == rhs);
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
