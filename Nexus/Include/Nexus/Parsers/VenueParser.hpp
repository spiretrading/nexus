#ifndef NEXUS_MARKET_PARSER_HPP
#define NEXUS_MARKET_PARSER_HPP
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /**
   * Parses a Venue.
   * @param venues The database of available venues to parse.
   */
  inline auto venue_parser(const VenueDatabase& venues) {
    auto code = [] (const auto& entry) {
      return entry.m_venue;
    };
    return Beam::EnumeratorParser(
      boost::make_transform_iterator(venues.get_entries().cbegin(), code),
      boost::make_transform_iterator(venues.get_entries().cend(), code),
      [=] (auto code) {
        return venues.from(code).m_display_name;
      });
  }

  /** Parses a MarketCode using the default MarketDatabase. */
  inline const auto& venue_parser() {
    static const auto& parser = venue_parser(DEFAULT_VENUES);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Venue> = Nexus::venue_parser();
}

#endif
