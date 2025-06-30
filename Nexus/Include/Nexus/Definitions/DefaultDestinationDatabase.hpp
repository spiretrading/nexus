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
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XASX"));
        entry.m_venues.push_back(Venue("XCNQ"));
        entry.m_venues.push_back(Venue("XHKG"));
        entry.m_venues.push_back(Venue("XNAS"));
        entry.m_venues.push_back(Venue("XNYS"));
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
        entry.m_id = "AMEX";
        entry.m_description = "NYSE MKT LLC";
        entry.m_venues.push_back(Venue("XASE"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "ARCA";
        entry.m_description = "NYSE ARCA";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "BATS";
        entry.m_description = "BATS Exchange";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "BATY";
        entry.m_description = "BATS Y-Exchange";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CBSX";
        entry.m_description = "CBOF Stock Exchange";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
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
        entry.m_id = "EDGA";
        entry.m_description = "EDGA Exchange";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "EDGX";
        entry.m_description = "EDGX Exchange";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "HKEX";
        entry.m_description = "Hong Kong Stock Exchange";
        entry.m_venues.push_back(Venue("XHKG"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "NYSE";
        entry.m_description = "NYSE";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "NASDAQ";
        entry.m_description = "NASDAQ";
        entry.m_venues.push_back(Venue("XASE"));
        entry.m_venues.push_back(Venue("XNYS"));
        entry.m_venues.push_back(Venue("XNAS"));
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
        entry.m_id = "OSE";
        entry.m_description = "Osaka Exchange";
        entry.m_venues.push_back(Venue("XOSE"));
        database.add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "TSE";
        entry.m_description = "Tokyo Stock Exchange";
        entry.m_venues.push_back(Venue("XTKS"));
        database.add(entry);
      }
      database.set_preferred_destination(Venue("XASE"), "NASDAQ");
      database.set_preferred_destination(Venue("XASX"), "ASXT");
      database.set_preferred_destination(Venue("XCNQ"), "CSE");
      database.set_preferred_destination(Venue("CSE2"), "CSE");
      database.set_preferred_destination(Venue("XNYS"), "NASDAQ");
      database.set_preferred_destination(Venue("XNAS"), "NASDAQ");
      database.set_preferred_destination(Venue("XTSE"), "TSX");
      database.set_preferred_destination(Venue("XTSX"), "TSX");
      database.set_preferred_destination(Venue("XOSE"), "OSE");
      database.set_preferred_destination(Venue("XTKS"), "TSE");
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
    inline const auto AMEX = std::string("AMEX");
    inline const auto CBSX = std::string("CBSX");
    inline const auto CXA = std::string("CXA");
    inline const auto ARCA = std::string("ARCA");
    inline const auto BATS = std::string("BATS");
    inline const auto BATY = std::string("BATY");
    inline const auto EDGA = std::string("EDGA");
    inline const auto EDGX = std::string("EDGX");
    inline const auto HKEX = std::string("HKEX");
    inline const auto NYSE = std::string("NYSE");
    inline const auto NASDAQ = std::string("NASDAQ");
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
    inline const auto OSE = std::string("OSE");
    inline const auto TSE = std::string("TSE");
  }
}

#endif
