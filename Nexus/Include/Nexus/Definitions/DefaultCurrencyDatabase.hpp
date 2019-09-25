#ifndef NEXUS_DEFAULTCURRENCYDATABASE_HPP
#define NEXUS_DEFAULTCURRENCYDATABASE_HPP
#include <memory>
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {
namespace Details {
  inline CurrencyDatabase BuildDefaultCurrencyDatabase() {
    CurrencyDatabase database;
    {
      CurrencyDatabase::Entry entry;
      entry.m_id = CurrencyId{36};
      entry.m_code = "AUD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      CurrencyDatabase::Entry entry;
      entry.m_id = CurrencyId{124};
      entry.m_code = "CAD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      CurrencyDatabase::Entry entry;
      entry.m_id = CurrencyId{344};
      entry.m_code = "HKD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      CurrencyDatabase::Entry entry;
      entry.m_id = CurrencyId{840};
      entry.m_code = "USD";
      entry.m_sign = "$";
      database.Add(entry);
    }
    {
      CurrencyDatabase::Entry entry;
      entry.m_id = CurrencyId{1001};
      entry.m_code = "XBT";
      entry.m_sign = "B";
      database.Add(entry);
    }
    return database;
  }
}

  //! Returns the default CurrencyDatabase, typically used for testing purposes.
  inline const CurrencyDatabase& GetDefaultCurrencyDatabase() {
    static auto database = std::make_unique<CurrencyDatabase>(
      Details::BuildDefaultCurrencyDatabase());
    return *database;
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
