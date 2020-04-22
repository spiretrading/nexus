#ifndef NEXUS_ORDER_TYPE_PARSER_HPP
#define NEXUS_ORDER_TYPE_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <Beam/Parsers/Types.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/OrderType.hpp"

namespace Nexus {

  /** Parses an OrderType. */
  inline const auto& OrderTypeParser() {
    static const auto parser = Beam::Parsers::EnumeratorParser(
      begin(Beam::MakeRange<OrderType>()),
      end(Beam::MakeRange<OrderType>()),
      &boost::lexical_cast<std::string, OrderType>);
    return parser;
  }
}

namespace Beam::Parsers {
  template<>
  const auto default_parser<Nexus::OrderType> = Nexus::OrderTypeParser();
}

#endif
