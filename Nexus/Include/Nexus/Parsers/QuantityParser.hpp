#ifndef NEXUS_QUANTITY_PARSER_HPP
#define NEXUS_QUANTITY_PARSER_HPP
#include <Beam/Parsers/DefaultParser.hpp>
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Returns a Quantity parser. */
  inline const auto& quantity_parser() {
    static const auto parser = Beam::DecimalParser<Quantity>();
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Quantity> = Nexus::quantity_parser();
}

#endif
