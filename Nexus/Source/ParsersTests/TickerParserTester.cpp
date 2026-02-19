#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/TickerParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("TickerParser") {
  TEST_CASE("well_formed_ticker") {
    auto parser = TickerParser();
    auto stream = to_parser_stream("ABX.TSX");
    auto ticker = Ticker();
    REQUIRE(parser.read(stream, ticker));
    REQUIRE(ticker.get_symbol() == "ABX");
    REQUIRE(ticker.get_venue() == DefaultVenues::TSX);
  }

  TEST_CASE("invalid_venue") {
    auto parser = TickerParser();
    auto stream = to_parser_stream("ABX.XST");
    auto ticker = Ticker();
    REQUIRE_FALSE(parser.read(stream, ticker));
  }

  TEST_CASE("lowercase_ticker") {
    auto parser = TickerParser();
    auto stream = to_parser_stream("aBx.TsX");
    auto ticker = Ticker();
    REQUIRE(parser.read(stream, ticker));
    REQUIRE(ticker.get_symbol() == "ABX");
    REQUIRE(ticker.get_venue() == DefaultVenues::TSX);
  }

  TEST_CASE("missing_dot") {
    auto parser = TickerParser();
    auto stream = to_parser_stream("ABXTSX");
    auto ticker = Ticker();
    REQUIRE_FALSE(parser.read(stream, ticker));
  }

  TEST_CASE("empty_symbol") {
    auto parser = TickerParser();
    auto stream = to_parser_stream(".TSX");
    auto ticker = Ticker();
    REQUIRE_FALSE(parser.read(stream, ticker));
  }

  TEST_CASE("empty_venue") {
    auto parser = TickerParser();
    auto stream = to_parser_stream("ABX.");
    auto ticker = Ticker();
    REQUIRE_FALSE(parser.read(stream, ticker));
  }
}
