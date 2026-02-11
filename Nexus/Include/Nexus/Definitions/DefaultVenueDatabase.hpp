#ifndef NEXUS_DEFINITIONS_DEFAULT_VENUE_DATABASE_HPP
#define NEXUS_DEFINITIONS_DEFAULT_VENUE_DATABASE_HPP
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {
namespace Details {
  inline const VenueDatabase& get_base_venue_database() {
    static auto database = [] {
      auto database = VenueDatabase();
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XASX");
        entry.m_country_code = DefaultCountries::AU;
        entry.m_market_center = "ASX";
        entry.m_time_zone = "Australia/Sydney";
        entry.m_currency = DefaultCurrencies::AUD;
        entry.m_description = "Australian Stock Market";
        entry.m_display_name = "ASX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("CHIA");
        entry.m_country_code = DefaultCountries::AU;
        entry.m_market_center = "CXA";
        entry.m_time_zone = "Australia/Sydney";
        entry.m_currency = DefaultCurrencies::AUD;
        entry.m_description = "CHI-X Australia";
        entry.m_display_name = "CXA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XATS");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Alpha Exchange";
        entry.m_display_name = "XATS";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("CHIC");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "CHI-X Canada ATS";
        entry.m_display_name = "CHIC";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XCNQ");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Canadian Securities Exchange";
        entry.m_display_name = "CSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("CSE2");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Canadian Securities Exchange - CSE2";
        entry.m_display_name = "CSE2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XCXD");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "NASDAQ CXD";
        entry.m_display_name = "CXD";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XCX2");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "CX2";
        entry.m_display_name = "CX2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("LYNX");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Lynx ATS";
        entry.m_display_name = "LYNX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("MATN");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "MATCH Now";
        entry.m_display_name = "MATN";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("NEOE");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_market_center = "AQL";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_display_name = "NEOE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("OMGA");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Omega ATS";
        entry.m_display_name = "OMGA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("PURE");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Pure Trading";
        entry.m_display_name = "PURE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XTSE");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_market_center = "TSE";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "Toronto Stock Exchange";
        entry.m_display_name = "TSX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XTSX");
        entry.m_country_code = DefaultCountries::CA;
        entry.m_market_center = "CDX";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = DefaultCurrencies::CAD;
        entry.m_description = "TSX Venture Exchange";
        entry.m_display_name = "TSXV";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("OTCM");
        entry.m_country_code = DefaultCountries::US;
        entry.m_market_center = "OTCM";
        entry.m_time_zone = "America/New_York";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "OTC Markets Group Inc.";
        entry.m_display_name = "OTCM";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("ASTR");
        entry.m_country_code = DefaultCountries::US;
        entry.m_market_center = "ASTR";
        entry.m_time_zone = "Etc/UTC";
        entry.m_currency = DefaultCurrencies::USD;
        entry.m_description = "Aster DEX";
        entry.m_display_name = "ASTR";
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  inline auto default_venues = get_base_venue_database();
}

  /** Returns the default VenueDatabase, typically used for testing purposes. */
  inline const VenueDatabase& DEFAULT_VENUES = Details::default_venues;

  /** Updates the default VenueDatabase. */
  inline void set_default_venues(VenueDatabase database) {
    Details::default_venues = database;
  }

  namespace DefaultVenues {
    inline const auto ASX = DEFAULT_VENUES.from("XASX").m_venue;
    inline const auto CXA = DEFAULT_VENUES.from("CHIA").m_venue;
    inline const auto CSE = DEFAULT_VENUES.from("XCNQ").m_venue;
    inline const auto CSE2 = DEFAULT_VENUES.from("CSE2").m_venue;
    inline const auto CHIC = DEFAULT_VENUES.from("CHIC").m_venue;
    inline const auto CXD = DEFAULT_VENUES.from("CXD").m_venue;
    inline const auto LYNX = DEFAULT_VENUES.from("LYNX").m_venue;
    inline const auto MATN = DEFAULT_VENUES.from("MATN").m_venue;
    inline const auto NEOE = DEFAULT_VENUES.from("NEOE").m_venue;
    inline const auto OMGA = DEFAULT_VENUES.from("OMGA").m_venue;
    inline const auto PURE = DEFAULT_VENUES.from("PURE").m_venue;
    inline const auto TSX = DEFAULT_VENUES.from("XTSE").m_venue;
    inline const auto TSXV = DEFAULT_VENUES.from("XTSX").m_venue;
    inline const auto XATS = DEFAULT_VENUES.from("XATS").m_venue;
    inline const auto XCX2 = DEFAULT_VENUES.from("XCX2").m_venue;
    inline const auto OTCM = DEFAULT_VENUES.from("OTCM").m_venue;
    inline const auto ASTR = DEFAULT_VENUES.from("ASTR").m_venue;
  }
}

#endif
