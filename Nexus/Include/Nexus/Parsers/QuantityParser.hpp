#ifndef NEXUS_QUANTITY_PARSER_HPP
#define NEXUS_QUANTITY_PARSER_HPP
#include <Beam/Parsers/Types.hpp>
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  //! Returns a Quantity parser.
  inline const auto& QuantityParser() {
    static const auto parser = Beam::Parsers::DecimalParser<Quantity>();
    return parser;
  }
}

namespace Beam::Parsers {
  template<>
  const auto default_parser<Nexus::Quantity> = Nexus::QuantityParser();
}

#endif
