#ifndef NEXUS_ORDERTYPEPARSER_HPP
#define NEXUS_ORDERTYPEPARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include "Nexus/Definitions/OrderType.hpp"

namespace Nexus {

  /*! \class OrderTypeParser
      \brief Matches an OrderType.
   */
  class OrderTypeParser : public Beam::Parsers::EnumeratorParser<OrderType> {
    public:

      //! Constructs a OrderTypeParser.
      OrderTypeParser();
  };

  inline OrderTypeParser::OrderTypeParser()
      : Beam::Parsers::EnumeratorParser<OrderType>(
          begin(Beam::MakeRange<OrderType>()),
          end(Beam::MakeRange<OrderType>()),
          static_cast<std::string (*)(OrderType)>(ToString)) {}
}

#endif
