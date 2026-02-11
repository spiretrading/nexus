#ifndef NEXUS_DEFINITIONS_DEFAULT_COUNTRY_DATABASE_HPP
#define NEXUS_DEFINITIONS_DEFAULT_COUNTRY_DATABASE_HPP
#include <atomic>
#include <memory>
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {
namespace Details {
  inline const CountryDatabase& get_base_country_database() {
    static auto database = [] {
      auto database = CountryDatabase();
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "Australia";
        entry.m_two_letter_code = "AU";
        entry.m_three_letter_code = "AUS";
        entry.m_code = CountryCode(36);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "Brazil";
        entry.m_two_letter_code = "BR";
        entry.m_three_letter_code = "BRA";
        entry.m_code = CountryCode(76);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "Canada";
        entry.m_two_letter_code = "CA";
        entry.m_three_letter_code = "CAN";
        entry.m_code = CountryCode(124);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "China";
        entry.m_two_letter_code = "CN";
        entry.m_three_letter_code = "CHN";
        entry.m_code = CountryCode(156);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "United Kingdom";
        entry.m_two_letter_code = "GB";
        entry.m_three_letter_code = "GBR";
        entry.m_code = CountryCode(826);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "Hong Kong";
        entry.m_two_letter_code = "HK";
        entry.m_three_letter_code = "HKG";
        entry.m_code = CountryCode(344);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "Japan";
        entry.m_two_letter_code = "JP";
        entry.m_three_letter_code = "JPN";
        entry.m_code = CountryCode(392);
        database.add(entry);
      }
      {
        auto entry = CountryDatabase::Entry();
        entry.m_name = "United States";
        entry.m_two_letter_code = "US";
        entry.m_three_letter_code = "USA";
        entry.m_code = CountryCode(840);
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  inline auto default_countries = get_base_country_database();
}

  /** Returns the default CountryDatabase. */
  inline const CountryDatabase& DEFAULT_COUNTRIES = Details::default_countries;

  /** Updates the default CountryDatabase. */
  inline void set_default_countries(CountryDatabase database) {
    Details::default_countries = database;
  }

  namespace DefaultCountries {
    inline const auto AU = DEFAULT_COUNTRIES.from("AU").m_code;
    inline const auto BR = DEFAULT_COUNTRIES.from("BR").m_code;
    inline const auto CA = DEFAULT_COUNTRIES.from("CA").m_code;
    inline const auto CN = DEFAULT_COUNTRIES.from("CN").m_code;
    inline const auto GB = DEFAULT_COUNTRIES.from("GB").m_code;
    inline const auto HK = DEFAULT_COUNTRIES.from("HK").m_code;
    inline const auto JP = DEFAULT_COUNTRIES.from("JP").m_code;
    inline const auto US = DEFAULT_COUNTRIES.from("US").m_code;
  }
}

#endif
