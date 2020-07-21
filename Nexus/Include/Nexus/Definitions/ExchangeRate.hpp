#ifndef NEXUS_EXCHANGE_RATE_HPP
#define NEXUS_EXCHANGE_RATE_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleRational.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/CurrencyPair.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Represents a CurrencyPair's exchange rate. */
  struct ExchangeRate {

    /** The CurrencyPair this exchange rate represents. */
    CurrencyPair m_pair;

    /** The conversion rate from the base currency to the counter currency. */
    boost::rational<int> m_rate;

    /** Constructs an ExchangeRate between two NONE currencies. */
    ExchangeRate();

    /**
     * Constructs an ExchangeRate.
     * @param pair The CurrencyPair to represent.
     * @param rate The conversion rate from the base currency to the counter
     *        currency.
     */
    ExchangeRate(CurrencyPair pair, boost::rational<int> rate);
  };

  /**
   * Switches the base currency with the counter currency in an ExchangeRate.
   * @param exchangeRate The ExchangeRate to invert.
   * @return The ExchangeRate with the base and counter inverted.
   */
  inline ExchangeRate Invert(const ExchangeRate& exchangeRate) {
    return {Invert(exchangeRate.m_pair),
      {exchangeRate.m_rate.denominator(), exchangeRate.m_rate.numerator()}};
  }

  /**
   * Converts a Money value according to an ExchangeRate.
   * @param value The value to convert.
   * @param exchangeRate The ExchangeRate to use in the conversion.
   * @return The <i>value</i> converted according to the specified
   *         <i>exchangeRate</i>.
   */
  inline Money Convert(Money value, const ExchangeRate& exchangeRate) {
    return (exchangeRate.m_rate.numerator() * value) /
      exchangeRate.m_rate.denominator();
  }

  inline ExchangeRate::ExchangeRate()
    : m_rate(1) {}

  inline ExchangeRate::ExchangeRate(CurrencyPair pair,
    boost::rational<int> rate)
    : m_pair(pair),
      m_rate(rate) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::ExchangeRate> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::ExchangeRate& value,
        unsigned int version) {
      shuttle.Shuttle("pair", value.m_pair);
      shuttle.Shuttle("rate", value.m_rate);
    }
  };
}

#endif
