#ifndef NEXUS_DEFINITIONS_EXCHANGE_RATE_HPP
#define NEXUS_DEFINITIONS_EXCHANGE_RATE_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleRational.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/CurrencyPair.hpp"
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Represents a CurrencyPair's exchange rate. */
  struct ExchangeRate {

    /** The CurrencyPair this exchange rate represents. */
    CurrencyPair m_pair;

    /** The conversion rate from the base currency to the counter currency. */
    boost::rational<int> m_rate;

    /** Constructs an ExchangeRate between two NONE currencies. */
    ExchangeRate() noexcept;

    /**
     * Constructs an ExchangeRate.
     * @param pair The CurrencyPair to represent.
     * @param rate The conversion rate from the base currency to the counter
     *        currency.
     */
    ExchangeRate(CurrencyPair pair, boost::rational<int> rate) noexcept;

    bool operator ==(const ExchangeRate&) const = default;
  };

  /**
   * Switches the base currency with the counter currency in an ExchangeRate.
   * @param rate The ExchangeRate to invert.
   * @return The ExchangeRate with the base and counter inverted.
   */
  inline ExchangeRate invert(const ExchangeRate& rate) {
    return ExchangeRate(invert(rate.m_pair),
      boost::rational<int>(rate.m_rate.denominator(), rate.m_rate.numerator()));
  }

  /**
   * Converts a Money value according to an ExchangeRate.
   * @param value The value to convert.
   * @param rate The ExchangeRate to use in the conversion.
   * @return The <i>value</i> converted according to the specified <i>rate</i>.
   */
  inline Money convert(Money value, const ExchangeRate& rate) {
    return (rate.m_rate.numerator() * value) / rate.m_rate.denominator();
  }

  inline std::ostream& operator<<(std::ostream& out, const ExchangeRate& rate) {
    return out << '(' << rate.m_pair << ' ' << rate.m_rate.numerator() << '/' <<
      rate.m_rate.denominator() << ')';
  }

  inline ExchangeRate::ExchangeRate() noexcept
    : m_rate(1) {}

  inline ExchangeRate::ExchangeRate(
    CurrencyPair pair, boost::rational<int> rate) noexcept
    : m_pair(pair),
      m_rate(rate) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ExchangeRate> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::ExchangeRate& value, unsigned int version) const {
      shuttle.shuttle("pair", value.m_pair);
      shuttle.shuttle("rate", value.m_rate);
    }
  };
}

#endif
