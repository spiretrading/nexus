#ifndef NEXUS_DEFAULT_CURRENCY_DATABASE_HPP
#define NEXUS_DEFAULT_CURRENCY_DATABASE_HPP
#include <memory>
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {
namespace Details {
  inline CurrencyDatabase BuildDefaultCurrencyDatabase() {
    auto database = CurrencyDatabase();
    {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(36);
      entry.m_code = "AUD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(124);
      entry.m_code = "CAD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(344);
      entry.m_code = "HKD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(392);
      entry.m_code = "JPY";
      entry.m_sign = "¥";
      database.Add(entry);
    }
    {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(840);
      entry.m_code = "USD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      auto entry = CurrencyDatabase::Entry();
      entry.m_id = CurrencyId(1001);
      entry.m_code = "XBT";
      entry.m_sign = "B";
      database.Add(entry);
    }
    return database;
  }
}

  /**
   * Returns the default CurrencyDatabase, typically used for testing purposes.
   */
  inline const CurrencyDatabase& GetDefaultCurrencyDatabase() {
    static auto database = Details::BuildDefaultCurrencyDatabase();
    return database;
  }

  /**
   * Parses a CurrencyId from a string.
   * @param source The string to parse.
   * @return The CurrencyId represented by the <i>source</i>.
   */
  inline CurrencyId ParseCurrency(const std::string& source) {
    return ParseCurrency(source, GetDefaultCurrencyDatabase());
  }

  namespace DefaultCurrencies {
    inline CurrencyId AUD() {
      static auto value = GetDefaultCurrencyDatabase().FromCode("AUD").m_id;
      return value;
    }

    inline CurrencyId CAD() {
      static auto value = GetDefaultCurrencyDatabase().FromCode("CAD").m_id;
      return value;
    }

    inline CurrencyId HKD() {
      static auto value = GetDefaultCurrencyDatabase().FromCode("HKD").m_id;
      return value;
    }

    inline CurrencyId JPY() {
      static auto value = GetDefaultCurrencyDatabase().FromCode("JPY").m_id;
      return value;
    }

    inline CurrencyId USD() {
      static auto value = GetDefaultCurrencyDatabase().FromCode("USD").m_id;
      return value;
    }

    inline CurrencyId XBT() {
      static auto value = GetDefaultCurrencyDatabase().FromCode("XBT").m_id;
      return value;
    }
  }
}

#endif
