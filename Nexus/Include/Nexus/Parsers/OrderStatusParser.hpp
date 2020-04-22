#ifndef NEXUS_ORDER_STATUS_PARSER_HPP
#define NEXUS_ORDER_STATUS_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <Beam/Parsers/Types.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"

namespace Nexus {

  /** Parses an OrderStatus. */
  inline const auto& OrderStatusParser() {
    static const auto parser = Beam::Parsers::EnumeratorParser(
      begin(Beam::MakeRange<OrderStatus>()),
      end(Beam::MakeRange<OrderStatus>()),
      &boost::lexical_cast<std::string, OrderStatus>);
    return parser;
  }
}

namespace Beam::Parsers {
  template<>
  const auto default_parser<Nexus::OrderStatus> = Nexus::OrderStatusParser();
}

#endif
