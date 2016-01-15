#ifndef NEXUS_SECURITYPARSER_HPP
#define NEXUS_SECURITYPARSER_HPP
#include <cctype>
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class SecurityParser
      \brief Matches a Security.
   */
  class SecurityParser : public Beam::Parsers::ParserOperators {
    public:
      typedef Security Result;

      //! Constructs a SecurityParser.
      /*!
        \param marketDatabase The database of markets to match against.
      */
      SecurityParser(const MarketDatabase& marketDatabase);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      MarketDatabase m_marketDatabase;
  };

  inline SecurityParser::SecurityParser(const MarketDatabase& marketDatabase)
      : m_marketDatabase(marketDatabase) {}

  template<typename ParserStreamType>
  bool SecurityParser::Read(ParserStreamType& source, Result& value) {
    Beam::Parsers::SubParserStream<ParserStreamType> context(source);
    std::string symbol;
    while(context.Read()) {
      if(std::isalnum(context.GetChar()) || context.GetChar() == '.') {
        symbol += std::toupper(context.GetChar());
      } else {
        context.Undo();
        break;
      }
    }
    value = ParseSecurity(symbol, m_marketDatabase);
    if(value.GetMarket() == MarketCode()) {
      return false;
    }
    context.Accept();
    return true;
  }

  template<typename ParserStreamType>
  bool SecurityParser::Read(ParserStreamType& source) {
    Security security;
    return Read(source, security);
  }
}

#endif
