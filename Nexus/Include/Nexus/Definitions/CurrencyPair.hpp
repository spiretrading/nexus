#ifndef NEXUS_DEFINITIONS_CURRENCY_PAIR_HPP
#define NEXUS_DEFINITIONS_CURRENCY_PAIR_HPP
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Asset.hpp"
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {

  /** Represents a pair of currencies. */
  struct CurrencyPair {

    /** The base currency. */
    Asset m_base;

    /** The counter currency. */
    Asset m_counter;

    auto operator <=>(const CurrencyPair&) const = default;
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
    if(separator == std::string_view::npos) {
      boost::throw_with_location(std::runtime_error("Missing separator '/'"));
    }
    auto base_code = symbol.substr(0, separator);
    auto counter_code = symbol.substr(separator + 1);
    auto base = database.from(base_code).m_id;
    if(!base) {
      boost::throw_with_location(
        std::runtime_error("Invalid base currency code."));
    }
    auto counter = database.from(counter_code).m_id;
    if(!counter) {
      boost::throw_with_location(
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
    extern const CurrencyDatabase& DEFAULT_CURRENCIES;
    return parse_currency_pair(symbol, DEFAULT_CURRENCIES);
  }

  /**
   * Switches the base currency with the counter currency in a CurrencyPair.
   * @param pair The CurrencyPair to invert.
   * @return The CurrencyPair with the base and counter inverted.
   */
  inline CurrencyPair invert(CurrencyPair pair) {
    return CurrencyPair(pair.m_counter, pair.m_base);
  }

  inline std::ostream& operator <<(std::ostream& out, CurrencyPair pair) {
    return out << pair.m_base << '/' << pair.m_counter;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::CurrencyPair> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::CurrencyPair& value, unsigned int version) const {
      shuttle.shuttle("base", value.m_base);
      shuttle.shuttle("counter", value.m_counter);
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::CurrencyPair> {
    size_t operator()(Nexus::CurrencyPair value) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed, std::hash<Nexus::Asset>()(value.m_base));
      boost::hash_combine(seed, std::hash<Nexus::Asset>()(value.m_counter));
      return seed;
    }
  };
}

#endif
