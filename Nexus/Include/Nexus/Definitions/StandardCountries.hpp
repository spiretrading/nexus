#ifndef NEXUS_STANDARD_COUNTRIES_HPP
#define NEXUS_STANDARD_COUNTRIES_HPP
#include <atomic>
#include <memory>
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {
namespace Details {
  inline const CountryDatabase& get_base_countries() {
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

  inline auto countries = get_base_countries();
}

  /** Returns the CountryDatabase. */
  inline const CountryDatabase& COUNTRIES = Details::countries;

  /** Updates the CountryDatabase. */
  inline void set_countries(CountryDatabase database) {
    Details::countries = database;
  }

  namespace Countries {
    inline const auto AU = COUNTRIES.from("AU").m_code;
    inline const auto BR = COUNTRIES.from("BR").m_code;
    inline const auto CA = COUNTRIES.from("CA").m_code;
    inline const auto CN = COUNTRIES.from("CN").m_code;
    inline const auto GB = COUNTRIES.from("GB").m_code;
    inline const auto HK = COUNTRIES.from("HK").m_code;
    inline const auto JP = COUNTRIES.from("JP").m_code;
    inline const auto US = COUNTRIES.from("US").m_code;
  }
}

#endif
