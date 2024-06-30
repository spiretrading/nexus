#ifndef NEXUS_SECURITYPARSER_HPP
#define NEXUS_SECURITYPARSER_HPP
#include <cctype>
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class SecurityParser
      \brief Matches a Security.
   */
  class SecurityParser {
    public:
      using Result = Security;

      //! Constructs a SecurityParser using the default MarketDatabase.
      SecurityParser();

      //! Constructs a SecurityParser.
      /*!
        \param marketDatabase The database of markets to match against.
      */
      SecurityParser(const MarketDatabase& marketDatabase);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      MarketDatabase m_marketDatabase;
  };

  inline SecurityParser::SecurityParser()
    : SecurityParser(GetDefaultMarketDatabase()) {}

  inline SecurityParser::SecurityParser(const MarketDatabase& marketDatabase)
    : m_marketDatabase(marketDatabase) {}

  template<typename Stream>
  bool SecurityParser::Read(Stream& source, Result& value) const {
    auto context = Beam::Parsers::SubParserStream<Stream>(source);
    auto symbol = std::string();
    while(context.Read()) {
      if(std::isalnum(context.GetChar()) || context.GetChar() == '.') {
        symbol += static_cast<char>(std::toupper(context.GetChar()));
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

  template<typename Stream>
  bool SecurityParser::Read(Stream& source) const {
    auto security = Security();
    return Read(source, security);
  }
}

#endif
