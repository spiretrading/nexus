#ifndef NEXUS_DEFINITIONS_DEFAULT_CURRENCY_DATABASE_HPP
#define NEXUS_DEFINITIONS_DEFAULT_CURRENCY_DATABASE_HPP
#include <atomic>
#include <memory>
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {
namespace Details {
  inline const CurrencyDatabase& get_base_currency_database() {
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
        entry.m_id = CurrencyId(978);
        entry.m_code = "EUR";
        entry.m_sign = "\xE2\x82\xAC";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(826);
        entry.m_code = "GBP";
        entry.m_sign = "\xC2\xA3";
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
        entry.m_sign = "\xC2\xA5";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(840);
        entry.m_code = "USD";
        entry.m_sign = "$";
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  inline auto default_currencies = get_base_currency_database();
}

  /** Returns the default CurrencyDatabase. */
  inline const CurrencyDatabase& DEFAULT_CURRENCIES =
    Details::default_currencies;

  /** Updates the default CurrencyDatabase. */
  inline void set_default_currencies(CurrencyDatabase database) {
    Details::default_currencies = database;
  }

  namespace DefaultCurrencies {
    inline const auto AUD = DEFAULT_CURRENCIES.from("AUD").m_id;
    inline const auto CAD = DEFAULT_CURRENCIES.from("CAD").m_id;
    inline const auto EUR = DEFAULT_CURRENCIES.from("EUR").m_id;
    inline const auto GBP = DEFAULT_CURRENCIES.from("GBP").m_id;
    inline const auto HKD = DEFAULT_CURRENCIES.from("HKD").m_id;
    inline const auto JPY = DEFAULT_CURRENCIES.from("JPY").m_id;
    inline const auto USD = DEFAULT_CURRENCIES.from("USD").m_id;
  }
}

#endif
