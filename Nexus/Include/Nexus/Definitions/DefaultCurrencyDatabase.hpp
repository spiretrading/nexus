#ifndef NEXUS_DEFAULT_CURRENCY_DATABASE_HPP
#define NEXUS_DEFAULT_CURRENCY_DATABASE_HPP
#include <memory>
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {

  /**
   * Returns the default CurrencyDatabase, typically used for testing purposes.
   */
  inline const CurrencyDatabase& GetDefaultCurrencyDatabase() {
    static auto database = [] {
      auto database = CurrencyDatabase();
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(36);
        entry.m_code = "AUD";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(124);
        entry.m_code = "CAD";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(344);
        entry.m_code = "HKD";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(392);
        entry.m_code = "JPY";
        entry.m_sign = reinterpret_cast<const char*>(u8"\u00A5");
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(840);
        entry.m_code = "USD";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(1001);
        entry.m_code = "XBT";
        entry.m_sign = reinterpret_cast<const char*>(u8"\u20BF");
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  /**
   * Parses a CurrencyId from a string.
   * @param source The string to parse.
   * @return The CurrencyId represented by the <i>source</i>.
   */
  inline CurrencyId parse_currency(std::string_view source) {
    return parse_currency(source, GetDefaultCurrencyDatabase());
  }

  namespace DefaultCurrencies {
    inline const auto AUD = GetDefaultCurrencyDatabase().from("AUD").m_id;
    inline const auto CAD = GetDefaultCurrencyDatabase().from("CAD").m_id;
    inline const auto HKD = GetDefaultCurrencyDatabase().from("HKD").m_id;
    inline const auto JPY = GetDefaultCurrencyDatabase().from("JPY").m_id;
    inline const auto USD = GetDefaultCurrencyDatabase().from("USD").m_id;
    inline const auto XBT = GetDefaultCurrencyDatabase().from("XBT").m_id;
  }
}

#endif
