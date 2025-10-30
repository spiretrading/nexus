#ifndef NEXUS_CURRENCY_PAIR_NOT_FOUND_EXCEPTION_HPP
#define NEXUS_CURRENCY_PAIR_NOT_FOUND_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus {

  /** Signals that an operation depending on a CurrencyPair failed. */
  class CurrencyPairNotFoundException : public boost::exception,
      public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a CurrencyPairNotFoundException. */
      CurrencyPairNotFoundException();
  };

  inline CurrencyPairNotFoundException::CurrencyPairNotFoundException()
    : CurrencyPairNotFoundException("Currency pair not found.") {}
}

#endif
