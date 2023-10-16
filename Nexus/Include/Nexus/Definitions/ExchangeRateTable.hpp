#ifndef NEXUS_EXCHANGE_RATE_TABLE_HPP
#define NEXUS_EXCHANGE_RATE_TABLE_HPP
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/Definitions/CurrencyPairNotFoundException.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"

namespace Nexus {

  /** Stores a table of ExchangeRates. */
  class ExchangeRateTable {
    public:

      /** Constructs an empty ExchangeRateTable. */
      ExchangeRateTable() = default;

      ExchangeRateTable(const ExchangeRateTable& table);

      ExchangeRateTable(ExchangeRateTable&& table);

      /**
       * Finds an ExchangeRate.
       * @param pair The ExchangeRate's CurrencyPair.
       * @return The ExchangeRate for the specified <i>pair</i>.
       */
      boost::optional<ExchangeRate> Find(CurrencyPair pair) const;

      /**
       * Converts a Money value from a base currency to its counter currency.
       * @param value The value to convert.
       * @param base The base currency that the <i>value</i> represents.
       * @param counter The counter currency to convert the <i>value</i> to.
       * @return The <i>value</i> converted from the <i>base</i> currency to the
       *         <i>counter</i> currency.
       */
      Money Convert(Money value, CurrencyId base, CurrencyId counter) const;

      /**
       * Adds an ExchangeRate to this table.
       * @param pair The ExchangeRate to add.
       */
      void Add(const ExchangeRate& exchangeRate);

      ExchangeRateTable& operator =(const ExchangeRateTable& table);

      ExchangeRateTable& operator =(ExchangeRateTable&& table);

    private:
      mutable boost::mutex m_mutex;
      mutable std::vector<ExchangeRate> m_exchangeRates;

      void ImmutableAdd(const ExchangeRate& exchangeRate) const;
  };

  inline ExchangeRateTable::ExchangeRateTable(const ExchangeRateTable& table) {
    auto lock = boost::lock_guard(table.m_mutex);
    m_exchangeRates = table.m_exchangeRates;
  }

  inline ExchangeRateTable::ExchangeRateTable(ExchangeRateTable&& table) {
    auto lock = boost::lock_guard(table.m_mutex);
    m_exchangeRates = std::move(table.m_exchangeRates);
  }

  inline boost::optional<ExchangeRate> ExchangeRateTable::Find(
      CurrencyPair pair) const {
    if(pair.m_base == pair.m_counter) {
      return ExchangeRate(pair, 1);
    }
    auto ExchangeRateSearch =
      [&] (CurrencyPair pair) {
        auto exchangeRateIterator = std::lower_bound(m_exchangeRates.begin(),
          m_exchangeRates.end(), pair,
          [] (auto& lhs, auto& rhs) {
            return lhs.m_pair < rhs;
          });
        if(exchangeRateIterator == m_exchangeRates.end() ||
            exchangeRateIterator->m_pair != pair) {
          return m_exchangeRates.end();
        }
        return exchangeRateIterator;
      };
    auto lock = boost::lock_guard(m_mutex);
    auto exchangeRateIterator = ExchangeRateSearch(pair);
    if(exchangeRateIterator == m_exchangeRates.end()) {
      exchangeRateIterator = ExchangeRateSearch(Invert(pair));
      if(exchangeRateIterator == m_exchangeRates.end()) {
        return boost::none;
      }
      auto invertedExchangeRate = Invert(*exchangeRateIterator);
      ImmutableAdd(invertedExchangeRate);
      return invertedExchangeRate;
    }
    return *exchangeRateIterator;
  }

  inline Money ExchangeRateTable::Convert(Money value, CurrencyId base,
      CurrencyId counter) const {
    auto exchangeRate = Find({base, counter});
    if(!exchangeRate.is_initialized()) {
      BOOST_THROW_EXCEPTION(CurrencyPairNotFoundException());
    }
    return Nexus::Convert(value, *exchangeRate);
  }

  inline void ExchangeRateTable::Add(const ExchangeRate& exchangeRate) {
    if(exchangeRate.m_pair.m_base == exchangeRate.m_pair.m_counter) {
      return;
    }
    auto lock = boost::lock_guard(m_mutex);
    ImmutableAdd(exchangeRate);
    ImmutableAdd(Invert(exchangeRate));
  }

  inline ExchangeRateTable& ExchangeRateTable::operator =(
      const ExchangeRateTable& table) {
    auto lockA = boost::lock_guard(table.m_mutex);
    auto lockB = boost::lock_guard(m_mutex);
    m_exchangeRates = table.m_exchangeRates;
    return *this;
  }

  inline ExchangeRateTable& ExchangeRateTable::operator =(
      ExchangeRateTable&& table) {
    auto lock = boost::lock_guard(table.m_mutex);
    auto lockB = boost::lock_guard(m_mutex);
    m_exchangeRates = std::move(table.m_exchangeRates);
    return *this;
  }

  inline void ExchangeRateTable::ImmutableAdd(
      const ExchangeRate& exchangeRate) const {
    auto exchangeRateIterator = std::lower_bound(m_exchangeRates.begin(),
        m_exchangeRates.end(), exchangeRate,
      [] (auto& lhs, auto& rhs) {
        return lhs.m_pair < rhs.m_pair;
      });
    if(exchangeRateIterator == m_exchangeRates.end()) {
      m_exchangeRates.push_back(exchangeRate);
    } else if(exchangeRateIterator->m_pair == exchangeRate.m_pair) {
      *exchangeRateIterator = exchangeRate;
    } else {
      m_exchangeRates.insert(exchangeRateIterator, exchangeRate);
    }
  }
}

#endif
