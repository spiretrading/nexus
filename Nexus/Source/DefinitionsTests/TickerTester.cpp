#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("Ticker") {
  TEST_CASE("default") {
    auto ticker = Ticker();
    REQUIRE(ticker.get_symbol().empty());
    REQUIRE(!ticker.get_venue());
  }

  TEST_CASE("symbol_constructor") {
    auto ticker = Ticker("S32");
    REQUIRE(ticker.get_symbol() == "S32");
    REQUIRE(!ticker.get_venue());
  }

  TEST_CASE("constructor") {
    auto ticker = Ticker("S32", ASX);
    REQUIRE(ticker.get_symbol() == "S32");
    REQUIRE(ticker.get_venue() == ASX);
  }

  TEST_CASE("empty") {
    REQUIRE_FALSE(static_cast<bool>(Ticker()));
    REQUIRE(static_cast<bool>(Ticker("S32", ASX)));
  }

  TEST_CASE("hash") {
    auto x = Ticker("SHOP", TSX);
    auto y = Ticker("SHOP", TSX);
    auto hasher = std::hash<Ticker>();
    REQUIRE(hasher(x) == hasher(y));
  }

  TEST_CASE("parse") {
    auto invalid = parse_ticker("XYZ", DEFAULT_VENUES);
    REQUIRE(!invalid);
    auto valid = parse_ticker("TD.TSX", DEFAULT_VENUES);
    REQUIRE(valid.get_symbol() == "TD");
    REQUIRE(valid.get_venue() == TSX);
    auto compound = parse_ticker("A.B.C.TSX", DEFAULT_VENUES);
    REQUIRE(compound.get_symbol() == "A.B.C");
    REQUIRE(compound.get_venue() == TSX);
  }

  TEST_CASE("stream") {
    auto ticker = Ticker("ARQ", TSXV);
    REQUIRE(to_string(ticker) == "ARQ.TSXV");
    auto empty = Ticker();
    REQUIRE(to_string(empty) == "");
    test_round_trip_shuttle(ticker);
  }

  TEST_CASE("stream_input") {
    auto ss = std::istringstream("ETHC.NEOE");
    auto parsed_ticker = Ticker();
    ss >> parsed_ticker;
    REQUIRE(parsed_ticker == Ticker("ETHC", NEOE));
    ss = std::istringstream("INVALID");
    auto invalid_ticker = Ticker();
    ss >> invalid_ticker;
    REQUIRE(!invalid_ticker);
  }

  TEST_CASE("parse_ticker_set") {
    auto yaml = R"(
      - ABX.TSX
      - RY.TSX
      - ETHC.NEOE
    )";
    auto node = YAML::Load(yaml);
    auto tickers = parse_ticker_set(node);
    REQUIRE(tickers.size() == 3);
    REQUIRE(tickers.contains(Ticker("ABX", TSX)));
    REQUIRE(tickers.contains(Ticker("RY", TSX)));
    REQUIRE(tickers.contains(Ticker("ETHC", NEOE)));
  }

  TEST_CASE("parse_ticker_set_with_invalid") {
    auto yaml = R"(
      - ABX.TSX
      - "   .TSX"
      - INVALID
      - RY.TSX
    )";
    auto node = YAML::Load(yaml);
    auto tickers = parse_ticker_set(node);
    REQUIRE(tickers.size() == 2);
    REQUIRE(tickers.contains(Ticker("ABX", TSX)));
    REQUIRE(tickers.contains(Ticker("RY", TSX)));
  }
}
