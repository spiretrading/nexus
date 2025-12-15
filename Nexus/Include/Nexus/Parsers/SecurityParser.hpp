#ifndef NEXUS_SECURITY_PARSER_HPP
#define NEXUS_SECURITY_PARSER_HPP
#include <cctype>
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/Parser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include "Nexus/Definitions/Security.hpp"

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

      template<Beam::IsParserStream S>
      bool read(S& source, Result& value) const;
      template<Beam::IsParserStream S>
      bool read(S& source) const;

    private:
      VenueDatabase m_venues;
  };

  inline SecurityParser::SecurityParser() noexcept
    : SecurityParser(DEFAULT_VENUES) {}

  inline SecurityParser::SecurityParser(const VenueDatabase& venues) noexcept
    : m_venues(venues) {}

  template<Beam::IsParserStream S>
  bool SecurityParser::read(S& source, Result& value) const {
    auto context = Beam::SubParserStream<S>(source);
    auto symbol = std::string();
    while(context.read()) {
      if(std::isalnum(context.peek()) || context.peek() == '.') {
        symbol += static_cast<char>(std::toupper(context.peek()));
      } else {
        context.undo();
        break;
      }
    }
    value = parse_security(symbol, m_venues);
    if(!value.get_venue()) {
      return false;
    }
    context.accept();
    return true;
  }

  template<Beam::IsParserStream S>
  bool SecurityParser::read(S& source) const {
    auto security = Security();
    return read(source, security);
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Security> = Nexus::SecurityParser();
}

#endif
