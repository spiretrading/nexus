#ifndef NEXUS_TIMEINFORCEPARSER_HPP
#define NEXUS_TIMEINFORCEPARSER_HPP
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class TimeInForceParser
      \brief Matches a TimeInForce.
   */
  class TimeInForceParser : public Beam::Parsers::ParserOperators {
    public:
      typedef TimeInForce Result;

      //! Constructs a TimeInForceParser.
      TimeInForceParser();

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  inline TimeInForceParser::TimeInForceParser() {}

  template<typename ParserStreamType>
  bool TimeInForceParser::Read(ParserStreamType& source, Result& value) {
    return false;
  }

  template<typename ParserStreamType>
  bool TimeInForceParser::Read(ParserStreamType& source) {
    return false;
  }
}

#endif
