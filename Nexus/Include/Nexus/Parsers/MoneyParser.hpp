#ifndef NEXUS_MONEY_PARSER_HPP
#define NEXUS_MONEY_PARSER_HPP
#include <Beam/Parsers/ConversionParser.hpp>
#include <Beam/Parsers/DefaultParser.hpp>
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {

  /** Parses a Money value. */
  inline const auto& money_parser() {
    static const auto parser = Beam::convert(Beam::double_p, [] (double value) {
      return Money(Quantity(value));
    });
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Money> = Nexus::money_parser();
}

#endif
