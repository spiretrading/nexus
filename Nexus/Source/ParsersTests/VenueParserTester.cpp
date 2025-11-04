#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/VenueParser.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("VenueParser") {
  TEST_CASE("all_venues") {
    auto parser = venue_parser();
    for(auto& entry : DEFAULT_VENUES.get_entries()) {
      auto stream = to_parser_stream(entry.m_display_name);
      auto venue = Venue();
      REQUIRE(parser.read(stream, venue));
      REQUIRE(venue == entry.m_venue);
    }
  }

  TEST_CASE("invalid_venue") {
    auto parser = venue_parser();
    auto stream = to_parser_stream("INVALID_VENUE");
    auto venue = Venue();
    REQUIRE_FALSE(parser.read(stream, venue));
  }

  TEST_CASE("lowercase_venue") {
    auto parser = venue_parser();
    for(auto& entry : DEFAULT_VENUES.get_entries()) {
      auto display_name = entry.m_display_name;
      auto lower_display_name = std::string();
      lower_display_name.reserve(display_name.size());
      for(auto c : display_name) {
        lower_display_name.push_back(
          std::tolower(static_cast<unsigned char>(c)));
      }
      auto stream = to_parser_stream(lower_display_name);
      auto venue = Venue();
      REQUIRE_FALSE(parser.read(stream, venue));
    }
  }
}
