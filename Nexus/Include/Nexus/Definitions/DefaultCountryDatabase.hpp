#ifndef NEXUS_DEFAULTCOUNTRYDATABASE_HPP
#define NEXUS_DEFAULTCOUNTRYDATABASE_HPP
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {
namespace Details {
  inline CountryDatabase BuildDefaultCountryDatabase() {
    CountryDatabase database;
    {
      CountryDatabase::Entry entry;
      entry.m_name = "Australia";
      entry.m_twoLetterCode = "AU";
      entry.m_threeLetterCode = "AUS";
      entry.m_code = 36;
      database.Add(entry);
    }
    {
      CountryDatabase::Entry entry;
      entry.m_name = "Brazil";
      entry.m_twoLetterCode = "BR";
      entry.m_threeLetterCode = "BRA";
      entry.m_code = 76;
      database.Add(entry);
    }
    {
      CountryDatabase::Entry entry;
      entry.m_name = "Canada";
      entry.m_twoLetterCode = "CA";
      entry.m_threeLetterCode = "CAN";
      entry.m_code = 124;
      database.Add(entry);
    }
    {
      CountryDatabase::Entry entry;
      entry.m_name = "China";
      entry.m_twoLetterCode = "CN";
      entry.m_threeLetterCode = "CHN";
      entry.m_code = 156;
      database.Add(entry);
    }
    {
      CountryDatabase::Entry entry;
      entry.m_name = "Hong Kong";
      entry.m_twoLetterCode = "HK";
      entry.m_threeLetterCode = "HKG";
      entry.m_code = 344;
      database.Add(entry);
    }
    {
      CountryDatabase::Entry entry;
      entry.m_name = "United States";
      entry.m_twoLetterCode = "US";
      entry.m_threeLetterCode = "USA";
      entry.m_code = 840;
      database.Add(entry);
    }
    return database;
  }
}

  //! Returns the default CountryDatabase, typically used for testing purposes.
  inline const CountryDatabase& GetDefaultCountryDatabase() {
    static std::unique_ptr<CountryDatabase> database(
      std::make_unique<CountryDatabase>(
      Details::BuildDefaultCountryDatabase()));
    return *database;
  }

  namespace DefaultCountries {
    inline CountryCode AU() {
      static CountryCode value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "AU").m_code;
      return value;
    }

    inline CountryCode BR() {
      static CountryCode value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "BR").m_code;
      return value;
    }

    inline CountryCode CA() {
      static CountryCode value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "CA").m_code;
      return value;
    }

    inline CountryCode CN() {
      static CountryCode value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "CN").m_code;
      return value;
    }

    inline CountryCode HK() {
      static CountryCode value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "HK").m_code;
      return value;
    }

    inline CountryCode US() {
      static CountryCode value = GetDefaultCountryDatabase().FromTwoLetterCode(
        "US").m_code;
      return value;
    }
  }
}

#endif
