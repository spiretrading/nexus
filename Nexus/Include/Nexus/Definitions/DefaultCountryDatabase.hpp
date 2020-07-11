#ifndef NEXUS_DEFAULT_COUNTRY_DATABASE_HPP
#define NEXUS_DEFAULT_COUNTRY_DATABASE_HPP
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {
namespace Details {
  inline CountryDatabase BuildDefaultCountryDatabase() {
    auto database = CountryDatabase();
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "Australia";
      entry.m_twoLetterCode = "AU";
      entry.m_threeLetterCode = "AUS";
      entry.m_code = CountryCode(36);
      database.Add(entry);
    }
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "Brazil";
      entry.m_twoLetterCode = "BR";
      entry.m_threeLetterCode = "BRA";
      entry.m_code = CountryCode(76);
      database.Add(entry);
    }
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "Canada";
      entry.m_twoLetterCode = "CA";
      entry.m_threeLetterCode = "CAN";
      entry.m_code = CountryCode(124);
      database.Add(entry);
    }
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "China";
      entry.m_twoLetterCode = "CN";
      entry.m_threeLetterCode = "CHN";
      entry.m_code = CountryCode(156);
      database.Add(entry);
    }
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "Hong Kong";
      entry.m_twoLetterCode = "HK";
      entry.m_threeLetterCode = "HKG";
      entry.m_code = CountryCode(344);
      database.Add(entry);
    }
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "Japan";
      entry.m_twoLetterCode = "JP";
      entry.m_threeLetterCode = "JPN";
      entry.m_code = CountryCode(392);
      database.Add(entry);
    }
    {
      auto entry = CountryDatabase::Entry();
      entry.m_name = "United States";
      entry.m_twoLetterCode = "US";
      entry.m_threeLetterCode = "USA";
      entry.m_code = CountryCode(840);
      database.Add(entry);
    }
    return database;
  }
}

  /**
   * Returns the default CountryDatabase, typically used for testing purposes.
   */
  inline const CountryDatabase& GetDefaultCountryDatabase() {
    static auto database = Details::BuildDefaultCountryDatabase();
    return database;
  }

  namespace DefaultCountries {
    inline CountryCode AU() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "AU").m_code;
      return value;
    }

    inline CountryCode BR() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "BR").m_code;
      return value;
    }

    inline CountryCode CA() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "CA").m_code;
      return value;
    }

    inline CountryCode CN() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "CN").m_code;
      return value;
    }

    inline CountryCode HK() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "HK").m_code;
      return value;
    }

    inline CountryCode JP() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "JP").m_code;
      return value;
    }

    inline CountryCode US() {
      static auto value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "US").m_code;
      return value;
    }
  }
}

#endif
