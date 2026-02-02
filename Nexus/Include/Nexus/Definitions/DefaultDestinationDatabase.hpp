#ifndef NEXUS_DEFAULT_DESTINATION_DATABASE_HPP
#define NEXUS_DEFAULT_DESTINATION_DATABASE_HPP
#include <atomic>
#include <memory>
#include "Nexus/Definitions/Destination.hpp"

namespace Nexus {
namespace Details {
  inline const DestinationDatabase& get_base_destination_database() {
    static auto database = [] {
      auto database = DestinationDatabase();
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "MOE";
        entry.m_description = "Manual Order Entry";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XASX"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.set_manual_order_entry_destination(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "ASXT";
        entry.m_description = "ASX TradeMatch";
        entry.m_venues.push_back(Venue("XASX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CXA";
        entry.m_description = "CHI-X Australia";
        entry.m_venues.push_back(Venue("XASX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "ALPHA";
        entry.m_description = "Alpha ATS";
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CHIX";
        entry.m_description = "Chi-X";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CSE";
        entry.m_description = "Canadian Securities Exchange";
        entry.m_venues.push_back(Venue("XCNQ"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CSE2";
        entry.m_description = "Canadian Securities Exchange - CSE2";
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CX2";
        entry.m_description = "Chi-X 2";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "LYNX";
        entry.m_description = "LYNX ATS";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "MATNLP";
        entry.m_description = "MATCH Now Liquidity Provider";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "MATNMF";
        entry.m_description = "MATCH Now Market Flow";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "NEOE";
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "OMEGA";
        entry.m_description = "Omega ATS";
        entry.m_venues.push_back(Venue("NEOE"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "PURE";
        entry.m_description = "Pure ATS";
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "TSX";
        entry.m_description = "Toronto Stock Exchange";
        entry.m_venues.push_back(Venue("XTSE"));
        entry.m_venues.push_back(Venue("XTSX"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "OTCM";
        entry.m_description = "OTC Markets Group Inc.";
        entry.m_venues.push_back(Venue("OTCM"));
        database.add(entry);
      }
      database.set_preferred_destination(Venue("CSE2"), "CSE");
      database.set_preferred_destination(Venue("NEOE"), "NEOE");
      database.set_preferred_destination(Venue("XASX"), "ASXT");
      database.set_preferred_destination(Venue("XCNQ"), "CSE");
      database.set_preferred_destination(Venue("XTSE"), "TSX");
      database.set_preferred_destination(Venue("XTSX"), "TSX");
      database.set_preferred_destination(Venue("OTCM"), "OTCM");
      return database;
    }();
    return database;
  }

  inline auto default_destinations = get_base_destination_database();
}

  /** Returns the default DestinationDatabase. */
  inline const auto& DEFAULT_DESTINATIONS = Details::default_destinations;

  /** Updates the default DestinationDatabase. */
  inline void set_default_destinations(DestinationDatabase database) {
    Details::default_destinations = database;
  }

  namespace DefaultDestinations {
    inline const auto MOE = std::string("MOE");
    inline const auto ASXT = std::string("ASXT");
    inline const auto CXA = std::string("CXA");
    inline const auto ALPHA = std::string("ALPHA");
    inline const auto CHIX = std::string("CHIX");
    inline const auto CSE = std::string("CSE");
    inline const auto CSE2 = std::string("CSE2");
    inline const auto CX2 = std::string("CX2");
    inline const auto LYNX = std::string("LYNX");
    inline const auto MATNLP = std::string("MATNLP");
    inline const auto MATNMF = std::string("MATNMF");
    inline const auto NEOE = std::string("NEOE");
    inline const auto OMEGA = std::string("OMEGA");
    inline const auto PURE = std::string("PURE");
    inline const auto TSX = std::string("TSX");
    inline const auto OTCM = std::string("OTCM");
  }
}

#endif
