#ifndef NEXUS_MARKET_PARSER_HPP
#define NEXUS_MARKET_PARSER_HPP
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Parses a Venue. */
  inline const auto& venue_parser() {
    static const auto parser = [&] {
      auto code = [] (const auto& entry) {
        return entry.m_venue;
      };
      return Beam::EnumeratorParser(
        boost::make_transform_iterator(VENUES.get_entries().cbegin(), code),
        boost::make_transform_iterator(VENUES.get_entries().cend(), code),
        [] (auto code) {
          return VENUES.from(code).m_display_name;
        });
    }();
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Venue> = Nexus::venue_parser();
}

#endif
