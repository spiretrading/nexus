#ifndef NEXUS_ORDER_TYPE_PARSER_HPP
#define NEXUS_ORDER_TYPE_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/OrderType.hpp"

namespace Nexus {

  /** Parses an OrderType. */
  inline const auto& order_type_parser() {
    static const auto parser = Beam::EnumeratorParser(
      begin(Beam::make_range<OrderType>()), end(Beam::make_range<OrderType>()),
      &boost::lexical_cast<std::string, OrderType>);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::OrderType> = Nexus::order_type_parser();
}

#endif
