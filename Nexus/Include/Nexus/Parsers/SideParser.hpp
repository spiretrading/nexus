#ifndef NEXUS_SIDE_PARSER_HPP
#define NEXUS_SIDE_PARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Parses a Side. */
  inline const auto& side_parser() {
    static const auto parser = Beam::EnumeratorParser(
      begin(Beam::make_range<Side>()), end(Beam::make_range<Side>()),
      &boost::lexical_cast<std::string, Side>);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Side> = Nexus::side_parser();
}

#endif
