#ifndef NEXUS_ORDER_STATUS_PARSER_HPP
#define NEXUS_ORDER_STATUS_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"

namespace Nexus {

  /** Parses an OrderStatus. */
  inline const auto& order_status_parser() {
    static const auto parser = Beam::EnumeratorParser(
      begin(Beam::make_range<OrderStatus>()),
      end(Beam::make_range<OrderStatus>()),
      &boost::lexical_cast<std::string, OrderStatus>);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::OrderStatus> = Nexus::order_status_parser();
}

#endif
