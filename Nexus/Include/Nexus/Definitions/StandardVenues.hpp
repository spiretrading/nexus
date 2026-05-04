#ifndef NEXUS_STANDARD_VENUES_HPP
#define NEXUS_STANDARD_VENUES_HPP
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {
namespace Details {
  inline const VenueDatabase& get_base_venues() {
    static auto database = [] {
      auto database = VenueDatabase();
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XASX");
        entry.m_country_code = Countries::AU;
        entry.m_market_center = "ASX";
        entry.m_time_zone = "Australia/Sydney";
        entry.m_currency = Currencies::AUD;
        entry.m_description = "Australian Stock Market";
        entry.m_display_name = "ASX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("CHIA");
        entry.m_country_code = Countries::AU;
        entry.m_market_center = "CXA";
        entry.m_time_zone = "Australia/Sydney";
        entry.m_currency = Currencies::AUD;
        entry.m_description = "CHI-X Australia";
        entry.m_display_name = "CXA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XATS");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "ALP";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Alpha Exchange";
        entry.m_display_name = "XATS";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("CHIC");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "CHI";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "CHI-X Canada ATS";
        entry.m_display_name = "CHIC";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XCNQ");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "CNQ";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Canadian Securities Exchange";
        entry.m_display_name = "CSE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("CSE2");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "CS2";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Canadian Securities Exchange - CSE2";
        entry.m_display_name = "CSE2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XCXD");
        entry.m_country_code = Countries::CA;
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "NASDAQ CXD";
        entry.m_display_name = "CXD";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XCX2");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "CHT";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "CX2";
        entry.m_display_name = "CX2";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("LYNX");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "LYX";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Lynx ATS";
        entry.m_display_name = "LYNX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("MATN");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "TCM";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "MATCH Now";
        entry.m_display_name = "MATN";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("NEOE");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "AQL";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Aequitas NEO Exchange";
        entry.m_display_name = "NEOE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("OMGA");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "OMG";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Omega ATS";
        entry.m_display_name = "OMGA";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("PURE");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "PUR";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Pure Trading";
        entry.m_display_name = "PURE";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XTSE");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "TSE";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "Toronto Stock Exchange";
        entry.m_display_name = "TSX";
        database.add(entry);
      }
      {
        auto entry = VenueDatabase::Entry();
        entry.m_venue = Venue("XTSX");
        entry.m_country_code = Countries::CA;
        entry.m_market_center = "CDX";
        entry.m_time_zone = "America/Toronto";
        entry.m_currency = Currencies::CAD;
        entry.m_description = "TSX Venture Exchange";
        entry.m_display_name = "TSXV";
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  inline auto venues = get_base_venues();
}

  /** Returns the default VenueDatabase, typically used for testing purposes. */
  inline const VenueDatabase& VENUES = Details::venues;

  /** Updates the default VenueDatabase. */
  inline void set_venues(VenueDatabase database) {
    Details::venues = database;
  }

  namespace Venues {
    inline const auto ASX = VENUES.from("XASX").m_venue;
    inline const auto CXA = VENUES.from("CHIA").m_venue;
    inline const auto CSE = VENUES.from("XCNQ").m_venue;
    inline const auto CSE2 = VENUES.from("CSE2").m_venue;
    inline const auto CHIC = VENUES.from("CHIC").m_venue;
    inline const auto CXD = VENUES.from("CXD").m_venue;
    inline const auto LYNX = VENUES.from("LYNX").m_venue;
    inline const auto MATN = VENUES.from("MATN").m_venue;
    inline const auto NEOE = VENUES.from("NEOE").m_venue;
    inline const auto OMGA = VENUES.from("OMGA").m_venue;
    inline const auto PURE = VENUES.from("PURE").m_venue;
    inline const auto TSX = VENUES.from("XTSE").m_venue;
    inline const auto TSXV = VENUES.from("XTSX").m_venue;
    inline const auto XATS = VENUES.from("XATS").m_venue;
    inline const auto XCX2 = VENUES.from("XCX2").m_venue;
  }
}

#endif
