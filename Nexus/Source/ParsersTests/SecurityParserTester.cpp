#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Parsers/SecurityParser.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Nexus;

TEST_SUITE("SecurityParser") {
  TEST_CASE("well_formed_security") {
    auto parser = SecurityParser(GetDefaultMarketDatabase());
    auto stream = ParserStreamFromString("ABX.TSX");
    auto security = Security();
    REQUIRE(parser.Read(stream, security));
    REQUIRE(security.GetSymbol() == "ABX");
    REQUIRE(security.GetMarket() == DefaultMarkets::TSX());
    REQUIRE(security.GetCountry() == DefaultCountries::CA());
    stream = ParserStreamFromString("ABX.XST");
    REQUIRE(!parser.Read(stream, security));
  }

  TEST_CASE("upper_case_security") {
    auto parser = SecurityParser(GetDefaultMarketDatabase());
    auto stream = ParserStreamFromString("aBx.TsX");
    auto security = Security();
    REQUIRE(parser.Read(stream, security));
    REQUIRE(security.GetSymbol() == "ABX");
    REQUIRE(security.GetMarket() == DefaultMarkets::TSX());
    REQUIRE(security.GetCountry() == DefaultCountries::CA());
  }
}
