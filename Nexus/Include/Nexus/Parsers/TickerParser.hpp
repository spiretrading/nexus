#ifndef NEXUS_TICKER_PARSER_HPP
#define NEXUS_TICKER_PARSER_HPP
#include <cctype>
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include "Nexus/Definitions/Ticker.hpp"

namespace Nexus {

  /** Matches a Ticker. */
  class TickerParser {
    public:
      using Result = Ticker;

      template<Beam::IsParserStream S>
      bool read(S& source, Result& value) const;
      template<Beam::IsParserStream S>
      bool read(S& source) const;
  };

  template<Beam::IsParserStream S>
  bool TickerParser::read(S& source, Result& value) const {
    auto context = Beam::SubParserStream<S>(source);
    auto symbol = std::string();
    while(context.read()) {
      if(std::isalnum(context.peek()) || context.peek() == '.') {
        symbol += static_cast<char>(std::toupper(context.peek()));
      } else {
        context.undo();
        break;
      }
    }
    value = parse_ticker(symbol);
    if(!value.get_venue()) {
      return false;
    }
    context.accept();
    return true;
  }

  template<Beam::IsParserStream S>
  bool TickerParser::read(S& source) const {
    auto ticker = Ticker();
    return read(source, ticker);
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Ticker> = Nexus::TickerParser();
}

#endif
