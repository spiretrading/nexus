#ifndef NEXUS_MONEY_PARSER_HPP
#define NEXUS_MONEY_PARSER_HPP
#include <Beam/Parsers/ConversionParser.hpp>
#include <Beam/Parsers/Types.hpp>
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Parses a Money value. */
  inline const auto& money_parser() {
    static const auto parser =
      Beam::Parsers::Convert(Beam::Parsers::double_p, [] (double value) {
        return Money(Quantity(value));
      });
    return parser;
  }
}

namespace Beam::Parsers {
  template<>
  const auto default_parser<Nexus::Money> = Nexus::money_parser();
}

#endif
