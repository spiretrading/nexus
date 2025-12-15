#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/CountryParser.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultCountries;

TEST_SUITE("CountryParser") {
  TEST_CASE("valid_country") {
    auto parser = country_parser();
    auto stream = to_parser_stream("CAN");
    auto country = CountryCode();
    REQUIRE(parser.read(stream, country));
    REQUIRE(country == CA);
  }

  TEST_CASE("invalid_country") {
    auto parser = country_parser();
    auto stream = to_parser_stream("QQQQQ");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.read(stream, country));
    REQUIRE(!country);
  }

  TEST_CASE("lowercase_country") {
    auto parser = country_parser();
    auto stream = to_parser_stream("can");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.read(stream, country));
    REQUIRE(!country);
  }

  TEST_CASE("partial_country") {
    auto parser = country_parser();
    auto stream = to_parser_stream("CA");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.read(stream, country));
    REQUIRE(!country);
  }

  TEST_CASE("multiple_countries") {
    auto parser = country_parser();
    auto stream = to_parser_stream("CANUSA");
    auto country = CountryCode();
    REQUIRE(parser.read(stream, country));
    REQUIRE(country == CA);
    REQUIRE(parser.read(stream, country));
    REQUIRE(country == US);
  }
}
