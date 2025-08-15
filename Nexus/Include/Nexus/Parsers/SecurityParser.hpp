#ifndef NEXUS_SECURITY_PARSER_HPP
#define NEXUS_SECURITY_PARSER_HPP
#include <cctype>
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include <Beam/Parsers/Types.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"

namespace Nexus {

  /** Matches a Security. */
  class SecurityParser {
    public:
      using Result = Security;

      /** Constructs a SecurityParser using the default venues. */
      SecurityParser() noexcept;

      /**
       * Constructs a SecurityParser.
       * @param venues The venues to parse.
       */
      explicit SecurityParser(const VenueDatabase& venues) noexcept;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      VenueDatabase m_venues;
  };

  inline SecurityParser::SecurityParser() noexcept
    : SecurityParser(DEFAULT_VENUES) {}

  inline SecurityParser::SecurityParser(const VenueDatabase& venues) noexcept
    : m_venues(venues) {}

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
    value = parse_security(symbol, m_venues);
    if(!value.get_venue()) {
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

namespace Beam::Parsers {
  template<>
  const auto default_parser<Nexus::Security> = Nexus::SecurityParser();
}

#endif
