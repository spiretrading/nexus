#ifndef NEXUS_SIDE_PARSER_HPP
#define NEXUS_SIDE_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <Beam/Parsers/Types.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Parses a Side. */
  inline const auto& SideParser() {
    static const auto parser = Beam::Parsers::EnumeratorParser(
      begin(Beam::MakeRange<Side>()), end(Beam::MakeRange<Side>()),
      &boost::lexical_cast<std::string, Side>);
    return parser;
  }
}

namespace Beam::Parsers {
  template<>
  const auto default_parser<Nexus::Side> = Nexus::SideParser();
}

#endif
