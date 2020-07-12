#ifndef NEXUS_CURRENCY_PAIR_NOT_FOUND_EXCEPTION_HPP
#define NEXUS_CURRENCY_PAIR_NOT_FOUND_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /** Signals that an operation depending on a CurrencyPair failed. */
  class CurrencyPairNotFoundException : public std::runtime_error,
      public boost::exception {
    public:

      /** Constructs a CurrencyPairNotFoundException. */
      CurrencyPairNotFoundException();
  };

  inline CurrencyPairNotFoundException::CurrencyPairNotFoundException()
    : std::runtime_error("Currency pair not found.") {}
}

#endif
