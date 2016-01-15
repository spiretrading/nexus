#ifndef NEXUS_EXCHANGERATETABLE_HPP
#define NEXUS_EXCHANGERATETABLE_HPP
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

  /*! \class ExchangeRateTable
      \brief Stores a table of ExchangeRates.
   */
  class ExchangeRateTable {
    public:

      //! Finds an ExchangeRate.
      /*!
        \param pair The ExchangeRate's CurrencyPair.
        \return The ExchangeRate for the specified <i>pair</i>.
      */
      boost::optional<ExchangeRate> Find(const CurrencyPair& pair) const;

      //! Converts a Money value from a base currency to its counter currency.
      /*!
        \param value The value to convert.
        \param base The base currency that the <i>value</i> represents.
        \param counter The counter currency to convert the <i>value</i> to.
        \return The <i>value</i> converted from the <i>base</i> currency to the
                <i>counter</i> currency.
      */
      Money Convert(Money value, CurrencyId base, CurrencyId counter) const;

      //! Adds an ExchangeRate to this table.
      /*!
        \param pair The ExchangeRate to add.
      */
      void Add(const ExchangeRate& exchangeRate);

    private:
      mutable boost::mutex m_mutex;
      mutable std::vector<ExchangeRate> m_exchangeRates;

      void ImmutableAdd(const ExchangeRate& exchangeRate) const;
  };

  inline boost::optional<ExchangeRate> ExchangeRateTable::Find(
      const CurrencyPair& pair) const {
    if(pair.m_base == pair.m_counter) {
      return ExchangeRate{pair, 1};
    }
    auto ExchangeRateSearch =
      [&] (const CurrencyPair& pair) {
        auto exchangeRateIterator = std::lower_bound(m_exchangeRates.begin(),
          m_exchangeRates.end(), pair,
          [] (const ExchangeRate& lhs, const CurrencyPair& rhs) {
            return std::tie(lhs.m_pair.m_base, lhs.m_pair.m_counter) <
              std::tie(rhs.m_base, rhs.m_counter);
          });
        if(exchangeRateIterator == m_exchangeRates.end() ||
            exchangeRateIterator->m_pair.m_base != pair.m_base ||
            exchangeRateIterator->m_pair.m_counter != pair.m_counter) {
          return m_exchangeRates.end();
        }
        return exchangeRateIterator;
      };
    boost::lock_guard<boost::mutex> lock(m_mutex);
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
    boost::lock_guard<boost::mutex> lock(m_mutex);
    ImmutableAdd(exchangeRate);
    ImmutableAdd(Invert(exchangeRate));
  }

  inline void ExchangeRateTable::ImmutableAdd(
      const ExchangeRate& exchangeRate) const {
    auto exchangeRateIterator = std::lower_bound(m_exchangeRates.begin(),
        m_exchangeRates.end(), exchangeRate,
      [] (const ExchangeRate& lhs, const ExchangeRate& rhs) {
        return std::tie(lhs.m_pair.m_base, lhs.m_pair.m_counter) <
          std::tie(rhs.m_pair.m_base, rhs.m_pair.m_counter);
      });
    if(exchangeRateIterator == m_exchangeRates.end()) {
      m_exchangeRates.push_back(exchangeRate);
    } else if(exchangeRateIterator->m_pair.m_base ==
        exchangeRate.m_pair.m_base && exchangeRateIterator->m_pair.m_counter ==
        exchangeRate.m_pair.m_counter) {
      *exchangeRateIterator = exchangeRate;
    } else {
      m_exchangeRates.insert(exchangeRateIterator, exchangeRate);
    }
  }
}

#endif
