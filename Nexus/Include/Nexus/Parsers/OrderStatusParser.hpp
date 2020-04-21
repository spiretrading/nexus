#ifndef NEXUS_ORDER_STATUS_PARSER_HPP
#define NEXUS_ORDER_STATUS_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"

namespace Nexus {

  /*! \class OrderStatusParser
      \brief Matches an OrderStatus.
   */
  class OrderStatusParser :
      public Beam::Parsers::EnumeratorParser<OrderStatus> {
    public:

      //! Constructs a OrderStatusParser.
      OrderStatusParser();
  };

  inline OrderStatusParser::OrderStatusParser()
    : Beam::Parsers::EnumeratorParser<OrderStatus>(
        begin(Beam::MakeRange<OrderStatus>()),
        end(Beam::MakeRange<OrderStatus>()),
        &boost::lexical_cast<std::string, OrderStatus>) {}
}

#endif
