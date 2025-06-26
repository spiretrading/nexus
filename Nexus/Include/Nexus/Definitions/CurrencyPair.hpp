#ifndef NEXUS_CURRENCY_PAIR_HPP
#define NEXUS_CURRENCY_PAIR_HPP
#include <stdexcept>
#include <string_view>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"

namespace Nexus {

  /* Represents a pair of currencies. */
  struct CurrencyPair {

    /** The base currency. */
    CurrencyId m_base;

    /** The counter currency. */
    CurrencyId m_counter;

    auto operator <=>(const CurrencyPair& rhs) const = default;
  };

  /**
   * Parses a string representing a CurrencyPair.
   * @param symbol The CurrencyPair's symbol.
   * @param database The database containing all available currencies.
   * @return The CurrencyPair represented by the <i>symbol</i>.
   */
  inline CurrencyPair parse_currency_pair(
      std::string_view symbol, const CurrencyDatabase& database) {
    auto separator = symbol.find('/');
    if(separator == std::string::npos) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Missing separator '/'"));
    }
    auto base_code = symbol.substr(0, separator);
    auto counter_code = symbol.substr(separator + 1);
    auto base = database.from(base_code).m_id;
    if(base == CurrencyId::NONE) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Invalid base currency code."));
    }
    auto counter = database.from(counter_code).m_id;
    if(counter == CurrencyId::NONE) {
      BOOST_THROW_EXCEPTION(
        std::runtime_error("Invalid counter currency code."));
    }
    return CurrencyPair(base, counter);
  }

  /**
   * Parses a string representing a CurrencyPair using the default
   * CurrencyDatabase.
   * @param symbol The CurrencyPair's symbol.
   * @return The CurrencyPair represented by the <i>symbol</i>.
   */
  inline CurrencyPair parse_currency_pair(std::string_view symbol) {
    return parse_currency_pair(symbol, DEFAULT_CURRENCIES);
  }

  /**
   * Switches the base currency with the counter currency in a CurrencyPair.
   * @param pair The CurrencyPair to invert.
   * @return The CurrencyPair with the base and counter inverted.
   */
  inline CurrencyPair invert(CurrencyPair pair) {
    return {pair.m_counter, pair.m_base};
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::CurrencyPair> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CurrencyPair& value,
        unsigned int version) {
      shuttle.Shuttle("base", value.m_base);
      shuttle.Shuttle("counter", value.m_counter);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::CurrencyPair> {
    size_t operator()(Nexus::CurrencyPair value) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed, value.m_base);
      boost::hash_combine(seed, value.m_counter);
      return seed;
    }
  };
}

#endif
