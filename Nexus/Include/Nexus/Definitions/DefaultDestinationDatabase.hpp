#ifndef NEXUS_DEFAULT_DESTINATION_DATABASE_HPP
#define NEXUS_DEFAULT_DESTINATION_DATABASE_HPP
#include "Nexus/Definitions/Destination.hpp"

namespace Nexus {

  /** Returns the default DestinationDatabase. */
  inline const DestinationDatabase& GetDefaultDestinationDatabase() {
    static auto database = [] {
      auto database = DestinationDatabase();
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "MOE";
        entry.m_description = "Manual Order Entry";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XASX");
        entry.m_markets.push_back("XCNQ");
        entry.m_markets.push_back("XHKG");
        entry.m_markets.push_back("XNAS");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.SetManualOrderEntryDestination(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "ASXT";
        entry.m_description = "ASX TradeMatch";
        entry.m_markets.push_back("XASX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "AMEX";
        entry.m_description = "NYSE MKT LLC";
        entry.m_markets.push_back("XASE");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "ARCA";
        entry.m_description = "NYSE ARCA";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "BATS";
        entry.m_description = "BATS Exchange";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "BATY";
        entry.m_description = "BATS Y-Exchange";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CBSX";
        entry.m_description = "CBOF Stock Exchange";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CXA";
        entry.m_description = "CHI-X Australia";
        entry.m_markets.push_back("XASX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "EDGA";
        entry.m_description = "EDGA Exchange";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "EDGX";
        entry.m_description = "EDGX Exchange";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "HKEX";
        entry.m_description = "Hong Kong Stock Exchange";
        entry.m_markets.push_back("XHKG");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "NYSE";
        entry.m_description = "NYSE";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "NASDAQ";
        entry.m_description = "NASDAQ";
        entry.m_markets.push_back("XASE");
        entry.m_markets.push_back("XNYS");
        entry.m_markets.push_back("XNAS");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "ALPHA";
        entry.m_description = "Alpha ATS";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CHIX";
        entry.m_description = "Chi-X";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CSE";
        entry.m_description = "Canadian Securities Exchange";
        entry.m_markets.push_back("XCNQ");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "CX2";
        entry.m_description = "Chi-X 2";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "LYNX";
        entry.m_description = "LYNX ATS";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "MATNLP";
        entry.m_description = "MATCH Now Liquidity Provider";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "MATNMF";
        entry.m_description = "MATCH Now Market Flow";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "NEOE";
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "OMEGA";
        entry.m_description = "Omega ATS";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "PURE";
        entry.m_description = "Pure ATS";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "TSX";
        entry.m_description = "Toronto Stock Exchange";
        entry.m_markets.push_back("XTSE");
        entry.m_markets.push_back("XTSX");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "OSE";
        entry.m_description = "Osaka Exchange";
        entry.m_markets.push_back("XOSE");
        database.Add(entry);
      }
      {
        auto entry = DestinationDatabase::Entry();
        entry.m_id = "TSE";
        entry.m_description = "Tokyo Stock Exchange";
        entry.m_markets.push_back("XTKS");
        database.Add(entry);
      }
      database.SetPreferredDesintation("XASE", "NASDAQ");
      database.SetPreferredDesintation("XASX", "ASXT");
      database.SetPreferredDesintation("XCNQ", "CSE");
      database.SetPreferredDesintation("CSE2", "CSE");
      database.SetPreferredDesintation("XNYS", "NASDAQ");
      database.SetPreferredDesintation("XNAS", "NASDAQ");
      database.SetPreferredDesintation("XTSE", "TSX");
      database.SetPreferredDesintation("XTSX", "TSX");
      database.SetPreferredDesintation("XOSE", "OSE");
      database.SetPreferredDesintation("XTKS", "TSE");
      return database;
    }();
    return database;
  }

  namespace DefaultDestinations {
    inline const std::string& MOE() {
      static auto value = std::string("MOE");
      return value;
    }

    inline const std::string& ASXT() {
      static auto value = std::string("ASXT");
      return value;
    }

    inline const std::string& AMEX() {
      static auto value = std::string("AMEX");
      return value;
    }

    inline const std::string& CBSX() {
      static auto value = std::string("CBSX");
      return value;
    }

    inline const std::string& CXA() {
      static auto value = std::string("CXA");
      return value;
    }

    inline const std::string& ARCA() {
      static auto value = std::string("ARCA");
      return value;
    }

    inline const std::string& BATS() {
      static auto value = std::string("BATS");
      return value;
    }

    inline const std::string& BATY() {
      static auto value = std::string("BATY");
      return value;
    }

    inline const std::string& EDGA() {
      static auto value = std::string("EDGA");
      return value;
    }

    inline const std::string& EDGX() {
      static auto value = std::string("EDGX");
      return value;
    }

    inline const std::string& HKEX() {
      static auto value = std::string("HKEX");
      return value;
    }

    inline const std::string& NYSE() {
      static auto value = std::string("NYSE");
      return value;
    }

    inline const std::string& NASDAQ() {
      static auto value = std::string("NASDAQ");
      return value;
    }

    inline const std::string& ALPHA() {
      static auto value = std::string("ALPHA");
      return value;
    }

    inline const std::string& CHIX() {
      static auto value = std::string("CHIX");
      return value;
    }

    inline const std::string& CSE() {
      static auto value = std::string("CSE");
      return value;
    }

    inline const std::string& CX2() {
      static auto value = std::string("CX2");
      return value;
    }

    inline const std::string& LYNX() {
      static auto value = std::string("LYNX");
      return value;
    }

    inline const std::string& MATNLP() {
      static auto value = std::string("MATNLP");
      return value;
    }

    inline const std::string& MATNMF() {
      static auto value = std::string("MATNMF");
      return value;
    }

    inline const std::string& NEOE() {
      static auto value = std::string("NEOE");
      return value;
    }

    inline const std::string& OMEGA() {
      static auto value = std::string("OMEGA");
      return value;
    }

    inline const std::string& PURE() {
      static auto value = std::string("PURE");
      return value;
    }

    inline const std::string& TSX() {
      static auto value = std::string("TSX");
      return value;
    }

    inline const std::string& OSE() {
      static auto value = std::string("OSE");
      return value;
    }

    inline const std::string& TSE() {
      static auto value = std::string("TSE");
      return value;
    }
  }
}

#endif
