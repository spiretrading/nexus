#ifndef NEXUS_ORDERSTATUSPARSER_HPP
#define NEXUS_ORDERSTATUSPARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
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
          static_cast<std::string (*)(OrderStatus)>(ToString)) {}
}

#endif
