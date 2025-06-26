#ifndef NEXUS_DEFAULT_VENUE_DATABASE_HPP
#define NEXUS_DEFAULT_VENUE_DATABASE_HPP
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /**
   * Returns the default VenueDatabase, typically used for testing purposes.
   */
  inline const VenueDatabase& GetDefaultVenueDatabase() {
    static auto database = [] {
      auto database = VenueDatabase();
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XASX";
        entry.m_country_code = DefaultCountries::AU;
        entry.m_time_zone = "Australian_Eastern_Standard_Time";
        entry.m_currency = DefaultCurrencies::AUD;
        entry.m_description = "Australian Stock Market";
        entry.m_display_name = "ASX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CHIA";
        entry.m_country_code = DefaultCountries::AU;
        entry.m_time_zone = "Australian_Eastern_Standard_Time";
        entry.m_currency = DefaultCurrencies::AUD;
        entry.m_description = "CHI-X Australia";
        entry.m_display_name = "CXA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XASE";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "NYSE MKT LLC";
        entry.m_display_name = "ASEX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "ARCX";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "NYSE ARCA";
        entry.m_display_name = "ARCX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "BATS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "BATS Exchange";
        entry.m_display_name = "BATS";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "BATY";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "BATS-Y Exchange";
        entry.m_display_name = "BATY";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XBOS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "NASDAQ OMX BX";
        entry.m_display_name = "BOSX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCBO";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "Chicago Board of Options Exchange";
        entry.m_display_name = "CBOE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCIS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "National Stock Exchange";
        entry.m_display_name = "NSEX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XADF";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "FINRA Alternative Display Facility";
        entry.m_display_name = "ADFX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XISX";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "International Securities Exchange";
        entry.m_display_name = "ISE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "EDGA";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "EDGA Exchange";
        entry.m_display_name = "EDGA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "EDGX";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "EDGX Exchange";
        entry.m_display_name = "EDGX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XNYS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "NYSE";
        entry.m_display_name = "NYSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XNAS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "NASDAQ";
        entry.m_display_name = "NSDQ";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XPHL";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "NASDAQ OMX PHLX";
        entry.m_display_name = "PHLX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XATS";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Alpha Exchange";
        entry.m_display_name = "XATS";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CHIC";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "CHI-X Canada ATS";
        entry.m_display_name = "CHIC";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCNQ";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Canadian Securities Exchange";
        entry.m_display_name = "CSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CSE2";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Canadian Securities Exchange - CSE2";
        entry.m_display_name = "CSE2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCX2";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "CX2";
        entry.m_display_name = "CX2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "LYNX";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Lynx ATS";
        entry.m_display_name = "LYNX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "MATN";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "MATCH Now";
        entry.m_display_name = "MATN";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "NEOE";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_display_name = "NEOE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "OMGA";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Omega ATS";
        entry.m_display_name = "OMGA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "PURE";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Pure Trading";
        entry.m_display_name = "PURE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XTSE";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Toronto Stock Exchange";
        entry.m_display_name = "TSX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XTSX";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "TSX Venture Exchange";
        entry.m_display_name = "TSXV";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XHKG";
        entry.m_country_code = DefaultCountries::HK;
        entry.m_time_zone = "Asia/Hong_Kong";
        entry.m_currency = DefaultCurrencies::HKD;
        entry.m_description = "Hong Kong Stock Exchange";
        entry.m_display_name = "HKEX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XFKA";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY;
        entry.m_description = "Fukuoka Stock Exchange";
        entry.m_display_name = "XFKA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XTKS";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY;
        entry.m_description = "Tokyo Stock Exchange";
        entry.m_display_name = "TSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XOSE";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY;
        entry.m_description = "Osaka Exchange";
        entry.m_display_name = "OSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XNGO";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY;
        entry.m_description = "Nagoya Stock Exchange";
        entry.m_display_name = "NSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XSAP";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY;
        entry.m_description = "Sapporo Securities Exchange";
        entry.m_display_name = "SSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CHIJ";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY;
        entry.m_description = "CHI-X Japan";
        entry.m_display_name = "CHIJ";
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  /**
   * Parses a Venue from a string using the default database.
   * @param source The string to parse.
   * @return The Venue represented by the <i>source</i>.
   */
  inline Venue parse_venue(std::string_view source) {
    return parse_venue(source, GetDefaultVenueDatabase());
  }

  namespace DefaultVenues {
    inline const auto HKEX = GetDefaultVenueDatabase().from("XHKG").m_venue;
    inline const auto ASX = GetDefaultVenueDatabase().from("XASX").m_venue;
    inline const auto CXA = GetDefaultVenueDatabase().from("CHIA").m_venue;
    inline const auto ASEX = GetDefaultVenueDatabase().from("XASE").m_venue;
    inline const auto ARCX = GetDefaultVenueDatabase().from("ARCX").m_venue;
    inline const auto BATS = GetDefaultVenueDatabase().from("BATS").m_venue;
    inline const auto BATY = GetDefaultVenueDatabase().from("BATY").m_venue;
    inline const auto BOSX = GetDefaultVenueDatabase().from("XBOS").m_venue;
    inline const auto CBOE = GetDefaultVenueDatabase().from("XCBO").m_venue;
    inline const auto CSE = GetDefaultVenueDatabase().from("XCNQ").m_venue;
    inline const auto CSE2 = GetDefaultVenueDatabase().from("CSE2").m_venue;
    inline const auto NSEX = GetDefaultVenueDatabase().from("XCIS").m_venue;
    inline const auto ADFX = GetDefaultVenueDatabase().from("XADF").m_venue;
    inline const auto ISE = GetDefaultVenueDatabase().from("XISX").m_venue;
    inline const auto EDGA = GetDefaultVenueDatabase().from("EDGA").m_venue;
    inline const auto EDGX = GetDefaultVenueDatabase().from("EDGX").m_venue;
    inline const auto PHLX = GetDefaultVenueDatabase().from("XPHL").m_venue;
    inline const auto CHIC = GetDefaultVenueDatabase().from("CHIC").m_venue;
    inline const auto LYNX = GetDefaultVenueDatabase().from("LYNX").m_venue;
    inline const auto NASDAQ = GetDefaultVenueDatabase().from("XNAS").m_venue;
    inline const auto NYSE = GetDefaultVenueDatabase().from("XNYS").m_venue;
    inline const auto MATN = GetDefaultVenueDatabase().from("MATN").m_venue;
    inline const auto NEOE = GetDefaultVenueDatabase().from("NEOE").m_venue;
    inline const auto OMGA = GetDefaultVenueDatabase().from("OMGA").m_venue;
    inline const auto PURE = GetDefaultVenueDatabase().from("PURE").m_venue;
    inline const auto TSX = GetDefaultVenueDatabase().from("XTSE").m_venue;
    inline const auto TSXV = GetDefaultVenueDatabase().from("XTSX").m_venue;
    inline const auto XATS = GetDefaultVenueDatabase().from("XATS").m_venue;
    inline const auto XCX2 = GetDefaultVenueDatabase().from("XCX2").m_venue;
    inline const auto XFKA = GetDefaultVenueDatabase().from("XFKA").m_venue;
    inline const auto TSE = GetDefaultVenueDatabase().from("XTKS").m_venue;
    inline const auto OSE = GetDefaultVenueDatabase().from("XOSE").m_venue;
    inline const auto NSE = GetDefaultVenueDatabase().from("XNGO").m_venue;
    inline const auto SSE = GetDefaultVenueDatabase().from("XSAP").m_venue;
    inline const auto CHIJ = GetDefaultVenueDatabase().from("CHIJ").m_venue;
  }
}

#endif
