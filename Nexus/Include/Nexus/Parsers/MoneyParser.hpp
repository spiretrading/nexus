#ifndef NEXUS_MONEYPARSER_HPP
#define NEXUS_MONEYPARSER_HPP
#include <Beam/Parsers/DecimalParser.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class MoneyParser
      \brief Matches a Money value.
   */
  class MoneyParser : public Beam::Parsers::ParserOperators {
    public:
      typedef Money Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool MoneyParser::Read(ParserStreamType& source, Result& value) {
    double rawValue;
    if(!Beam::Parsers::DecimalParser<double>().Read(source, rawValue)) {
      return false;
    }
    value = Money{Quantity{rawValue}};
    return true;
  }

  template<typename ParserStreamType>
  bool MoneyParser::Read(ParserStreamType& source) {
    return Beam::Parsers::DecimalParser<double>().Read(source);
  }
}

#endif
