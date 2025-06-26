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
        entry.m_currency = DefaultCurrencies::AUD();
        entry.m_description = "Australian Stock Market";
        entry.m_display_name = "ASX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CHIA";
        entry.m_country_code = DefaultCountries::AU;
        entry.m_time_zone = "Australian_Eastern_Standard_Time";
        entry.m_currency = DefaultCurrencies::AUD();
        entry.m_description = "CHI-X Australia";
        entry.m_display_name = "CXA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XASE";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NYSE MKT LLC";
        entry.m_display_name = "ASEX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "ARCX";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NYSE ARCA";
        entry.m_display_name = "ARCX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "BATS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "BATS Exchange";
        entry.m_display_name = "BATS";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "BATY";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "BATS-Y Exchange";
        entry.m_display_name = "BATY";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XBOS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NASDAQ OMX BX";
        entry.m_display_name = "BOSX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCBO";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "Chicago Board of Options Exchange";
        entry.m_display_name = "CBOE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCIS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "National Stock Exchange";
        entry.m_display_name = "NSEX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XADF";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "FINRA Alternative Display Facility";
        entry.m_display_name = "ADFX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XISX";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "International Securities Exchange";
        entry.m_display_name = "ISE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "EDGA";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "EDGA Exchange";
        entry.m_display_name = "EDGA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "EDGX";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "EDGX Exchange";
        entry.m_display_name = "EDGX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XNYS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NYSE";
        entry.m_display_name = "NYSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XNAS";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NASDAQ";
        entry.m_display_name = "NSDQ";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XPHL";
        entry.m_country_code = DefaultCountries::US;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::USD();
        entry.m_description = "NASDAQ OMX PHLX";
        entry.m_display_name = "PHLX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XATS";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Alpha Exchange";
        entry.m_display_name = "XATS";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CHIC";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "CHI-X Canada ATS";
        entry.m_display_name = "CHIC";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCNQ";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Canadian Securities Exchange";
        entry.m_display_name = "CSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CSE2";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Canadian Securities Exchange - CSE2";
        entry.m_display_name = "CSE2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XCX2";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "CX2";
        entry.m_display_name = "CX2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "LYNX";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Lynx ATS";
        entry.m_display_name = "LYNX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "MATN";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "MATCH Now";
        entry.m_display_name = "MATN";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "NEOE";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_display_name = "NEOE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "OMGA";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Omega ATS";
        entry.m_display_name = "OMGA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "PURE";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Pure Trading";
        entry.m_display_name = "PURE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XTSE";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "Toronto Stock Exchange";
        entry.m_display_name = "TSX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XTSX";
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "Eastern_Time";
        entry.m_currency = DefaultCurrencies::CAD();
        entry.m_description = "TSX Venture Exchange";
        entry.m_display_name = "TSXV";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XHKG";
        entry.m_country_code = DefaultCountries::HK;
        entry.m_time_zone = "Asia/Hong_Kong";
        entry.m_currency = DefaultCurrencies::HKD();
        entry.m_description = "Hong Kong Stock Exchange";
        entry.m_display_name = "HKEX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XFKA";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Fukuoka Stock Exchange";
        entry.m_display_name = "XFKA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XTKS";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Tokyo Stock Exchange";
        entry.m_display_name = "TSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XOSE";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Osaka Exchange";
        entry.m_display_name = "OSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XNGO";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Nagoya Stock Exchange";
        entry.m_display_name = "NSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "XSAP";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
        entry.m_description = "Sapporo Securities Exchange";
        entry.m_display_name = "SSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = "CHIJ";
        entry.m_country_code = DefaultCountries::JP;
        entry.m_time_zone = "Asia/Tokyo";
        entry.m_currency = DefaultCurrencies::JPY();
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
    inline Venue HKEX() {
      static auto value = GetDefaultVenueDatabase().from("XHKG").m_venue;
      return value;
    }

    inline Venue ASX() {
      static auto value = GetDefaultVenueDatabase().from("XASX").m_venue;
      return value;
    }

    inline Venue CXA() {
      static auto value = GetDefaultVenueDatabase().from("CHIA").m_venue;
      return value;
    }

    inline Venue ASEX() {
      static auto value = GetDefaultVenueDatabase().from("XASE").m_venue;
      return value;
    }

    inline Venue ARCX() {
      static auto value = GetDefaultVenueDatabase().from("ARCX").m_venue;
      return value;
    }

    inline Venue BATS() {
      static auto value = GetDefaultVenueDatabase().from("BATS").m_venue;
      return value;
    }

    inline Venue BATY() {
      static auto value = GetDefaultVenueDatabase().from("BATY").m_venue;
      return value;
    }

    inline Venue BOSX() {
      static auto value = GetDefaultVenueDatabase().from("XBOS").m_venue;
      return value;
    }

    inline Venue CBOE() {
      static auto value = GetDefaultVenueDatabase().from("XCBO").m_venue;
      return value;
    }

    inline Venue CSE() {
      static auto value = GetDefaultVenueDatabase().from("XCNQ").m_venue;
      return value;
    }

    inline Venue CSE2() {
      static auto value = GetDefaultVenueDatabase().from("CSE2").m_venue;
      return value;
    }

    inline Venue NSEX() {
      static auto value = GetDefaultVenueDatabase().from("XCIS").m_venue;
      return value;
    }

    inline Venue ADFX() {
      static auto value = GetDefaultVenueDatabase().from("XADF").m_venue;
      return value;
    }

    inline Venue ISE() {
      static auto value = GetDefaultVenueDatabase().from("XISX").m_venue;
      return value;
    }

    inline Venue EDGA() {
      static auto value = GetDefaultVenueDatabase().from("EDGA").m_venue;
      return value;
    }

    inline Venue EDGX() {
      static auto value = GetDefaultVenueDatabase().from("EDGX").m_venue;
      return value;
    }

    inline Venue PHLX() {
      static auto value = GetDefaultVenueDatabase().from("XPHL").m_venue;
      return value;
    }

    inline Venue CHIC() {
      static auto value = GetDefaultVenueDatabase().from("CHIC").m_venue;
      return value;
    }

    inline Venue LYNX() {
      static auto value = GetDefaultVenueDatabase().from("LYNX").m_venue;
      return value;
    }

    inline Venue NASDAQ() {
      static auto value = GetDefaultVenueDatabase().from("XNAS").m_venue;
      return value;
    }

    inline Venue NYSE() {
      static auto value = GetDefaultVenueDatabase().from("XNYS").m_venue;
      return value;
    }

    inline Venue MATN() {
      static auto value = GetDefaultVenueDatabase().from("MATN").m_venue;
      return value;
    }

    inline Venue NEOE() {
      static auto value = GetDefaultVenueDatabase().from("NEOE").m_venue;
      return value;
    }

    inline Venue OMGA() {
      static auto value = GetDefaultVenueDatabase().from("OMGA").m_venue;
      return value;
    }

    inline Venue PURE() {
      static auto value = GetDefaultVenueDatabase().from("PURE").m_venue;
      return value;
    }

    inline Venue TSX() {
      static auto value = GetDefaultVenueDatabase().from("XTSE").m_venue;
      return value;
    }

    inline Venue TSXV() {
      static auto value = GetDefaultVenueDatabase().from("XTSX").m_venue;
      return value;
    }

    inline Venue XATS() {
      static auto value = GetDefaultVenueDatabase().from("XATS").m_venue;
      return value;
    }

    inline Venue XCX2() {
      static auto value = GetDefaultVenueDatabase().from("XCX2").m_venue;
      return value;
    }

    inline Venue XFKA() {
      static auto value = GetDefaultVenueDatabase().from("XFKA").m_venue;
      return value;
    }

    inline Venue TSE() {
      static auto value = GetDefaultVenueDatabase().from("XTKS").m_venue;
      return value;
    }

    inline Venue OSE() {
      static auto value = GetDefaultVenueDatabase().from("XOSE").m_venue;
      return value;
    }

    inline Venue NSE() {
      static auto value = GetDefaultVenueDatabase().from("XNGO").m_venue;
      return value;
    }

    inline Venue SSE() {
      static auto value = GetDefaultVenueDatabase().from("XSAP").m_venue;
      return value;
    }

    inline Venue CHIJ() {
      static auto value = GetDefaultVenueDatabase().from("CHIJ").m_venue;
      return value;
    }
  }
}

#endif
