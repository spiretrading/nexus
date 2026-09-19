#ifndef ASX_TRADE_ITCH_PARSER_EXCEPTION_HPP
#define ASX_TRADE_ITCH_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Reports an ASX Trade ITCH parsing failure. */
  class AsxTradeItchParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an AsxTradeItchParserException with a default message. */
      AsxTradeItchParserException();
  };

  inline AsxTradeItchParserException::AsxTradeItchParserException()
    : AsxTradeItchParserException("ASX Trade ITCH parser failed.") {}
}

#endif
