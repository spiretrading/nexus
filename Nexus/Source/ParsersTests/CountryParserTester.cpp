#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/CountryParser.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Nexus;

TEST_SUITE("CountryParser") {
  TEST_CASE("valid_country") {
    auto parser = CountryParser();
    auto stream = ParserStreamFromString("CAN");
    auto country = CountryCode();
    REQUIRE(parser.Read(stream, country));
    REQUIRE(country == DefaultCountries::CA());
  }

  TEST_CASE("invalid_country") {
    auto parser = CountryParser();
    auto stream = ParserStreamFromString("QQQQQ");
    auto country = CountryCode();
    REQUIRE_FALSE(parser.Read(stream, country));
    REQUIRE(country == CountryCode());
  }
}
