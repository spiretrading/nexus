#ifndef NEXUS_DEFAULT_COUNTRY_DATABASE_HPP
#define NEXUS_DEFAULT_COUNTRY_DATABASE_HPP
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {

  /**
   * Returns the default CountryDatabase, typically used for testing purposes.
   */
  inline const CountryDatabase& GetDefaultCountryDatabase() {
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

  /**
   * Parses a CountryCode from a string using the default database.
   * @param source The string to parse.
   * @return The CountryCode represented by the <i>source</i>.
   */
  inline CountryCode parse_country_code(std::string_view source) {
    return parse_country_code(source, GetDefaultCountryDatabase());
  }

  namespace DefaultCountries {
    inline const auto AU = GetDefaultCountryDatabase().from("AU").m_code;
    inline const auto BR = GetDefaultCountryDatabase().from("BR").m_code;
    inline const auto CA = GetDefaultCountryDatabase().from("CA").m_code;
    inline const auto CN = GetDefaultCountryDatabase().from("CN").m_code;
    inline const auto HK = GetDefaultCountryDatabase().from("HK").m_code;
    inline const auto JP = GetDefaultCountryDatabase().from("JP").m_code;
    inline const auto US = GetDefaultCountryDatabase().from("US").m_code;
  }
}

#endif
