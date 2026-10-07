#ifndef NEXUS_QUANTITY_PARSER_HPP
#define NEXUS_QUANTITY_PARSER_HPP
#include <Beam/Parsers/DefaultParser.hpp>
#include "Nexus/Definitions/Quantity.hpp"

namespace Beam {
  template<>
  inline bool parse_decimal<Nexus::Quantity>(
      std::string_view text, Nexus::Quantity& value) {
    auto buffer = std::string(text);
    auto end = buffer.find_first_of("eE");
    if(end == std::string::npos) {
      end = buffer.size();
    }
    auto point = buffer.find('.');
    if(point == std::string::npos) {
      point = end;
    } else {
      buffer.erase(point, 1);
      --end;
    }
    auto decimals = end - point;
    if(decimals > Nexus::Quantity::DECIMAL_PLACES) {
      buffer.insert(point + Nexus::Quantity::DECIMAL_PLACES, 1, '.');
    } else {
      buffer.insert(end, Nexus::Quantity::DECIMAL_PLACES - decimals, '0');
    }
    auto representation = double();
    if(!parse_decimal(buffer, representation)) {
      return false;
    }
    value = Nexus::Quantity::from_representation(representation);
    return true;
  }
}

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
