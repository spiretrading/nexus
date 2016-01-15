#ifndef NEXUS_CURRENCYPAIR_HPP
#define NEXUS_CURRENCYPAIR_HPP
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \struct CurrencyPair
      \brief Represents a single CurrencyPair.
  */
  struct CurrencyPair {

    //! The base currency.
    CurrencyId m_base;

    //! The counter currency.
    CurrencyId m_counter;

    //! Constructs a CurrencyPair between two NONE currencies.
    CurrencyPair() = default;

    //! Constructs a CurrencyPair.
    /*!
      \param base The base currency.
      \param counter The counter currency.
      \param conversion The conversion from the base currency to the counter
             currency.
    */
    CurrencyPair(CurrencyId base, CurrencyId counter);
  };

  //! Parses a string representing a CurrencyPair.
  /*!
    \param symbol The CurrencyPair's symbol.
    \param database The database containing all available currencies.
    \return The CurrencyPair represented by the <i>symbol</i>.
  */
  inline CurrencyPair ParseCurrencyPair(const std::string& symbol,
      const CurrencyDatabase& database) {
    auto separator = symbol.find('/');
    if(separator == std::string::npos) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Missing separator '/'"));
    }
    auto baseCode = symbol.substr(0, separator);
    auto counterCode = symbol.substr(separator + 1);
    auto base = database.FromCode(baseCode).m_id;
    if(base == CurrencyId::NONE()) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Invalid base currency code."));
    }
    auto counter = database.FromCode(counterCode).m_id;
    if(counter == CurrencyId::NONE()) {
      BOOST_THROW_EXCEPTION(std::runtime_error(
        "Invalid counter currency code."));
    }
    CurrencyPair currencyPair(base, counter);
    return currencyPair;
  }

  //! Parses a string representing a CurrencyPair using the default
  //! CurrencyDatabase.
  /*!
    \param symbol The CurrencyPair's symbol.
    \return The CurrencyPair represented by the <i>symbol</i>.
  */
  inline CurrencyPair ParseCurrencyPair(const std::string& symbol) {
    return ParseCurrencyPair(symbol, GetDefaultCurrencyDatabase());
  }

  //! Switches the base currency with the counter currency in a CurrencyPair.
  /*!
    \param pair The CurrencyPair to invert.
    \return The CurrencyPair with the base and counter inverted.
  */
  inline CurrencyPair Invert(const CurrencyPair& pair) {
    return {pair.m_counter, pair.m_base};
  }

  inline CurrencyPair::CurrencyPair(CurrencyId base, CurrencyId counter)
      : m_base{base},
        m_counter{counter} {}
}

namespace Beam {
namespace Serialization {
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
}

#endif
