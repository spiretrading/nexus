#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/CountryParser.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Nexus;
using namespace Nexus::DefaultCountries;

TEST_SUITE("CountryParser") {
  TEST_CASE("valid_country") {
    auto parser = country_parser();
    auto stream = ParserStreamFromString("CAN");
    auto country = CountryCode();
    REQUIRE(parser.Read(stream, country));
    REQUIRE(country == CA);
  }

  TEST_CASE("invalid_country") {
    auto parser = country_parser();
    auto stream = ParserStreamFromString("QQQQQ");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.Read(stream, country));
    REQUIRE(country == CountryCode::NONE);
  }

  TEST_CASE("lowercase_country") {
    auto parser = country_parser();
    auto stream = ParserStreamFromString("can");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.Read(stream, country));
    REQUIRE(country == CountryCode::NONE);
  }

  TEST_CASE("partial_country") {
    auto parser = country_parser();
    auto stream = ParserStreamFromString("CA");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.Read(stream, country));
    REQUIRE(country == CountryCode::NONE);
  }

  TEST_CASE("multiple_countries") {
    auto parser = country_parser();
    auto stream = ParserStreamFromString("CANUSA");
    auto country = CountryCode();
    REQUIRE(parser.Read(stream, country));
    REQUIRE(country == CA);
    REQUIRE(parser.Read(stream, country));
    REQUIRE(country == US);
  }
}
