#ifndef NEXUS_STANDARD_CURRENCIES_HPP
#define NEXUS_STANDARD_CURRENCIES_HPP
#include <atomic>
#include <memory>
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {
namespace Details {
  inline const CurrencyDatabase& get_base_currencies() {
    static auto database = [] {
      auto database = CurrencyDatabase();
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(36);
        entry.m_code = "AUD";
        entry.m_name = "Australian Dollar";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(124);
        entry.m_code = "CAD";
        entry.m_name = "Canadian Dollar";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(978);
        entry.m_code = "EUR";
        entry.m_name = "Euro";
        entry.m_sign = "\xE2\x82\xAC";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(826);
        entry.m_code = "GBP";
        entry.m_name = "Pound Sterling";
        entry.m_sign = "\xC2\xA3";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(344);
        entry.m_code = "HKD";
        entry.m_name = "Hong Kong Dollar";
        entry.m_sign = "$";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(392);
        entry.m_code = "JPY";
        entry.m_name = "Yen";
        entry.m_sign = "\xC2\xA5";
        database.add(entry);
      }
      {
        auto entry = CurrencyDatabase::Entry();
        entry.m_id = CurrencyId(840);
        entry.m_code = "USD";
        entry.m_name = "US Dollar";
        entry.m_sign = "$";
        database.add(entry);
      }
      return database;
    }();
    return database;
  }

  inline auto currencies = get_base_currencies();
}

  /** Returns the CurrencyDatabase. */
  inline const CurrencyDatabase& CURRENCIES = Details::currencies;

  /** Updates the CurrencyDatabase. */
  inline void set_currencies(CurrencyDatabase database) {
    Details::currencies = database;
  }

  namespace Currencies {
    inline const auto AUD = CURRENCIES.from("AUD").m_id;
    inline const auto CAD = CURRENCIES.from("CAD").m_id;
    inline const auto EUR = CURRENCIES.from("EUR").m_id;
    inline const auto GBP = CURRENCIES.from("GBP").m_id;
    inline const auto HKD = CURRENCIES.from("HKD").m_id;
    inline const auto JPY = CURRENCIES.from("JPY").m_id;
    inline const auto USD = CURRENCIES.from("USD").m_id;
  }
}

#endif
