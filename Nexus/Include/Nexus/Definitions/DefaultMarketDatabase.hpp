#ifndef NEXUS_DEFAULT_MARKET_DATABASE_HPP
#define NEXUS_DEFAULT_MARKET_DATABASE_HPP
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"

namespace Nexus {

  /**
   * Returns the default MarketDatabase, typically used for testing purposes.
   */
  inline const MarketDatabase& GetDefaultMarketDatabase() {
    static auto database = [] {
      auto database = MarketDatabase();
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XASX";
        entry.m_countryCode = DefaultCountries::AU();
        entry.m_timeZone = "Australian_Eastern_Standard_Time";
        entry.m_currency = DefaultCurrencies::AUD();
        entry.m_description = "Australian Stock Market";
        entry.m_displayName = "ASX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "CHIA";
        entry.m_countryCode = DefaultCountries::AU();
        entry.m_timeZone = "Australian_Eastern_Standard_Time";
        entry.m_currency = DefaultCurrencies::AUD();
        entry.m_description = "CHI-X Australia";
        entry.m_displayName = "CXA";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XASE";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NYSE MKT LLC";
        entry.m_displayName = "ASEX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "ARCX";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NYSE ARCA";
        entry.m_displayName = "ARCX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "BATS";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "BATS Exchange";
        entry.m_displayName = "BATS";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "BATY";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "BATS-Y Exchange";
        entry.m_displayName = "BATY";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XBOS";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NASDAQ OMX BX";
        entry.m_displayName = "BOSX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XCBO";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "Chicago Board of Options Exchange";
        entry.m_displayName = "CBOE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XCIS";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "National Stock Exchange";
        entry.m_displayName = "NSEX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XADF";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "FINRA Alternative Display Facility";
        entry.m_displayName = "ADFX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XISX";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "International Securities Exchange";
        entry.m_displayName = "ISE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "EDGA";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "EDGA Exchange";
        entry.m_displayName = "EDGA";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "EDGX";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "EDGX Exchange";
        entry.m_displayName = "EDGX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XNYS";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NYSE";
        entry.m_displayName = "NYSE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XNAS";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NASDAQ";
        entry.m_displayName = "NSDQ";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XPHL";
        entry.m_countryCode = DefaultCountries::US();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NASDAQ OMX PHLX";
        entry.m_displayName = "PHLX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XATS";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Alpha Exchange";
        entry.m_displayName = "XATS";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "CHIC";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "CHI-X Canada ATS";
        entry.m_displayName = "CHIC";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XCNQ";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Canadian Securities Exchange";
        entry.m_displayName = "CSE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "CSE2";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Canadian Securities Exchange - CSE2";
        entry.m_displayName = "CSE2";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XCX2";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "CX2";
        entry.m_displayName = "CX2";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XCXD";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "NASDAQ CXD";
        entry.m_displayName = "CXD";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "LYNX";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Lynx ATS";
        entry.m_displayName = "LYNX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "MATN";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "MATCH Now";
        entry.m_displayName = "MATN";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "NEOE";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_displayName = "NEOE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "OMGA";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Omega ATS";
        entry.m_displayName = "OMGA";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "PURE";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Pure Trading";
        entry.m_displayName = "PURE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XTSE";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Toronto Stock Exchange";
        entry.m_displayName = "TSX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XTSX";
        entry.m_countryCode = DefaultCountries::CA();
        entry.m_timeZone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "TSX Venture Exchange";
        entry.m_displayName = "TSXV";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XHKG";
        entry.m_countryCode = DefaultCountries::HK();
        entry.m_timeZone = "Asia/Hong_Kong";
        entry.m_currency = DefaultCurrencies::HKD();
        entry.m_description = "Hong Kong Stock Exchange";
        entry.m_displayName = "HKEX";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XFKA";
        entry.m_countryCode = DefaultCountries::JP();
        entry.m_timeZone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Fukuoka Stock Exchange";
        entry.m_displayName = "XFKA";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XTKS";
        entry.m_countryCode = DefaultCountries::JP();
        entry.m_timeZone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Tokyo Stock Exchange";
        entry.m_displayName = "TSE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XOSE";
        entry.m_countryCode = DefaultCountries::JP();
        entry.m_timeZone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Osaka Exchange";
        entry.m_displayName = "OSE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XNGO";
        entry.m_countryCode = DefaultCountries::JP();
        entry.m_timeZone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Nagoya Stock Exchange";
        entry.m_displayName = "NSE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "XSAP";
        entry.m_countryCode = DefaultCountries::JP();
        entry.m_timeZone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Sapporo Securities Exchange";
        entry.m_displayName = "SSE";
        database.Add(entry);
      }
      {
        auto entry = MarketDatabase::Entry();
        entry.m_code = "CHIJ";
        entry.m_countryCode = DefaultCountries::JP();
        entry.m_timeZone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "CHI-X Japan";
        entry.m_displayName = "CHIJ";
        database.Add(entry);
      }
      return database;
    }();
    return database;
  }

  /**
   * Parses a MarketCode from a string using the default database.
   * @param source The string to parse.
   * @return The MarketCode represented by the <i>source</i>.
   */
  inline MarketCode ParseMarketCode(const std::string& source) {
    return ParseMarketCode(source, GetDefaultMarketDatabase());
  }

  namespace DefaultMarkets {
    inline MarketCode HKEX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XHKG").m_code;
      return value;
    }

    inline MarketCode ASX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XASX").m_code;
      return value;
    }

    inline MarketCode CXA() {
      static auto value = GetDefaultMarketDatabase().FromCode("CHIA").m_code;
      return value;
    }

    inline MarketCode ASEX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XASE").m_code;
      return value;
    }

    inline MarketCode ARCX() {
      static auto value = GetDefaultMarketDatabase().FromCode("ARCX").m_code;
      return value;
    }

    inline MarketCode BATS() {
      static auto value = GetDefaultMarketDatabase().FromCode("BATS").m_code;
      return value;
    }

    inline MarketCode BATY() {
      static auto value = GetDefaultMarketDatabase().FromCode("BATY").m_code;
      return value;
    }

    inline MarketCode BOSX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XBOS").m_code;
      return value;
    }

    inline MarketCode CBOE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XCBO").m_code;
      return value;
    }

    inline MarketCode CSE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XCNQ").m_code;
      return value;
    }

    inline MarketCode CSE2() {
      static auto value = GetDefaultMarketDatabase().FromCode("CSE2").m_code;
      return value;
    }

    inline MarketCode NSEX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XCIS").m_code;
      return value;
    }

    inline MarketCode ADFX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XADF").m_code;
      return value;
    }

    inline MarketCode ISE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XISX").m_code;
      return value;
    }

    inline MarketCode EDGA() {
      static auto value = GetDefaultMarketDatabase().FromCode("EDGA").m_code;
      return value;
    }

    inline MarketCode EDGX() {
      static auto value = GetDefaultMarketDatabase().FromCode("EDGX").m_code;
      return value;
    }

    inline MarketCode PHLX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XPHL").m_code;
      return value;
    }

    inline MarketCode CHIC() {
      static auto value = GetDefaultMarketDatabase().FromCode("CHIC").m_code;
      return value;
    }

    inline MarketCode CXD() {
      static auto value = GetDefaultMarketDatabase().FromCode("XCXD").m_code;
      return value;
    }

    inline MarketCode LYNX() {
      static auto value = GetDefaultMarketDatabase().FromCode("LYNX").m_code;
      return value;
    }

    inline MarketCode NASDAQ() {
      static auto value = GetDefaultMarketDatabase().FromCode("XNAS").m_code;
      return value;
    }

    inline MarketCode NYSE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XNYS").m_code;
      return value;
    }

    inline MarketCode MATN() {
      static auto value = GetDefaultMarketDatabase().FromCode("MATN").m_code;
      return value;
    }

    inline MarketCode NEOE() {
      static auto value = GetDefaultMarketDatabase().FromCode("NEOE").m_code;
      return value;
    }

    inline MarketCode OMGA() {
      static auto value = GetDefaultMarketDatabase().FromCode("OMGA").m_code;
      return value;
    }

    inline MarketCode PURE() {
      static auto value = GetDefaultMarketDatabase().FromCode("PURE").m_code;
      return value;
    }

    inline MarketCode TSX() {
      static auto value = GetDefaultMarketDatabase().FromCode("XTSE").m_code;
      return value;
    }

    inline MarketCode TSXV() {
      static auto value = GetDefaultMarketDatabase().FromCode("XTSX").m_code;
      return value;
    }

    inline MarketCode XATS() {
      static auto value = GetDefaultMarketDatabase().FromCode("XATS").m_code;
      return value;
    }

    inline MarketCode XCX2() {
      static auto value = GetDefaultMarketDatabase().FromCode("XCX2").m_code;
      return value;
    }

    inline MarketCode XFKA() {
      static auto value = GetDefaultMarketDatabase().FromCode("XFKA").m_code;
      return value;
    }

    inline MarketCode TSE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XTKS").m_code;
      return value;
    }

    inline MarketCode OSE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XOSE").m_code;
      return value;
    }

    inline MarketCode NSE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XNGO").m_code;
      return value;
    }

    inline MarketCode SSE() {
      static auto value = GetDefaultMarketDatabase().FromCode("XSAP").m_code;
      return value;
    }

    inline MarketCode CHIJ() {
      static auto value = GetDefaultMarketDatabase().FromCode("CHIJ").m_code;
      return value;
    }
  }
}

#endif
