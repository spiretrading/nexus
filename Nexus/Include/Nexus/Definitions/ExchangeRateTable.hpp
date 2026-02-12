#ifndef NEXUS_DEFINITIONS_EXCHANGE_RATE_TABLE_HPP
#define NEXUS_DEFINITIONS_EXCHANGE_RATE_TABLE_HPP
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/CurrencyPair.hpp"
#include "Nexus/Definitions/CurrencyPairNotFoundException.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Stores and retrieves exchange rates for currency conversions. */
  class ExchangeRateTable {
    public:

      /** Constructs an empty table. */
      ExchangeRateTable() = default;

      /** Constructs a table from a list of exchange rates. */
      explicit ExchangeRateTable(const std::vector<ExchangeRate>& rates);

      ExchangeRateTable(const ExchangeRateTable& table);

      /**
       * Finds the exchange rate for a given currency pair.
       * @param pair The currency pair to look up.
       * @return The exchange rate if found or boost::none if no rate exists for
       *         the pair.
       */
      boost::optional<ExchangeRate> find(CurrencyPair pair) const;

      /**
       * Converts an amount using the exchange rate for a given currency pair.
       * @param value The monetary amount to convert.
       * @param pair The currency pair specifying base and counter currencies.
       * @return The converted monetary amount.
       * @throws CurrencyPairNotFoundException If no exchange rate exists for
       *         the pair.
       */
      Money convert(Money value, CurrencyPair pair) const;

      /**
       * Converts an amount using base and counter currency IDs.
       * @param value The monetary amount to convert.
       * @param base The base currency ID.
       * @param counter The counter currency ID.
       * @return The converted monetary amount.
       * @throws CurrencyPairNotFoundException If no exchange rate exists for
       *         the constructed pair.
       */
      Money convert(Money value, Asset base, Asset counter) const;

      /**
       * Adds or updates an exchange rate in the table.
       * @param rate The exchange rate to add or update.
       */
      void add(const ExchangeRate& rate);

    private:
      friend struct Beam::Shuttle<ExchangeRateTable>;
      mutable boost::mutex m_mutex;
      std::unordered_map<CurrencyPair, ExchangeRate> m_direct_rates;
      mutable std::unordered_map<CurrencyPair, ExchangeRate> m_derived_rates;
  };

  inline ExchangeRateTable::ExchangeRateTable(
      const std::vector<ExchangeRate>& rates) {
    for(auto& rate : rates) {
      add(rate);
    }
  }

  inline ExchangeRateTable::ExchangeRateTable(const ExchangeRateTable& table) {
    auto lock = boost::lock_guard(table.m_mutex);
    m_direct_rates = table.m_direct_rates;
    m_derived_rates = table.m_derived_rates;
  }

  inline boost::optional<ExchangeRate>
      ExchangeRateTable::find(CurrencyPair pair) const {
    {
      auto lock = boost::lock_guard(m_mutex);
      auto i = m_direct_rates.find(pair);
      if(i != m_direct_rates.end()) {
        return i->second;
      }
      auto j = m_derived_rates.find(pair);
      if(j != m_derived_rates.end()) {
        return j->second;
      }
    }
    if(pair.m_base == pair.m_counter) {
      return ExchangeRate(pair, 1);
    }
    auto rates = std::unordered_map<CurrencyPair, ExchangeRate>();
    {
      auto lock = boost::lock_guard(m_mutex);
      rates = m_direct_rates;
    }
    auto visited = std::unordered_set<Asset>();
    auto queue = std::queue<std::pair<Asset, boost::rational<int>>>();
    queue.push(std::pair(pair.m_base, boost::rational<int>(1)));
    visited.insert(pair.m_base);
    while(!queue.empty()) {
      auto [currency, cumulative_rate] = queue.front();
      queue.pop();
      if(currency == pair.m_counter) {
        auto derived_rate = ExchangeRate(pair, cumulative_rate);
        {
          auto lock = boost::lock_guard(m_mutex);
          m_derived_rates.insert(std::pair(pair, derived_rate));
        }
        return derived_rate;
      }
      for(auto& [pair, rate] : rates) {
        if(pair.m_base == currency) {
          auto next = pair.m_counter;
          if(visited.insert(next).second) {
            queue.push(std::pair(next, cumulative_rate * rate.m_rate));
          }
        } else if(pair.m_counter == currency) {
          auto next = pair.m_base;
          if(visited.insert(next).second) {
            auto inverse = boost::rational<int>(
              rate.m_rate.denominator(), rate.m_rate.numerator());
            queue.push(std::pair(next, cumulative_rate * inverse));
          }
        }
      }
    }
    return boost::none;
  }

  inline Money ExchangeRateTable::convert(
      Money value, CurrencyPair pair) const {
    if(auto rate = find(pair)) {
      return Nexus::convert(value, *rate);
    }
    boost::throw_with_location(CurrencyPairNotFoundException());
  }

  inline Money ExchangeRateTable::convert(
      Money value, Asset base, Asset counter) const {
    return convert(value, CurrencyPair(base, counter));
  }

  inline void ExchangeRateTable::add(const ExchangeRate& rate) {
    auto lock = boost::lock_guard(m_mutex);
    m_direct_rates[rate.m_pair] = rate;
    auto inverse = invert(rate);
    m_direct_rates[inverse.m_pair] = inverse;
    m_derived_rates.clear();
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ExchangeRateTable> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ExchangeRateTable& value,
        unsigned int version) const {
      auto lock = boost::lock_guard(value.m_mutex);
      shuttle.shuttle("direct_rates", value.m_direct_rates);
      if constexpr(IsReceiver<S>) {
        value.m_derived_rates.clear();
      }
    }
  };
}

#endif
