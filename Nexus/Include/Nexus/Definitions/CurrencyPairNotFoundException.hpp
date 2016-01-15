#ifndef NEXUS_CURRENCYPAIRNOTFOUNDEXCEPTION_HPP
#define NEXUS_CURRENCYPAIRNOTFOUNDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \class CurrencyPairNotFoundException
      \brief Signals that an operation depending on a CurrencyPair failed.
   */
  class CurrencyPairNotFoundException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a CurrencyPairNotFoundException.
      CurrencyPairNotFoundException();
  };

  inline CurrencyPairNotFoundException::CurrencyPairNotFoundException()
      : std::runtime_error("Currency pair not found.") {}
}

#endif
