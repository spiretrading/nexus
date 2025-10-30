#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Security.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("Security") {
  TEST_CASE("default") {
    auto security = Security();
    REQUIRE(security.get_symbol().empty());
    REQUIRE(!security.get_venue());
  }

  TEST_CASE("constructor") {
    auto security = Security("S32", ASX);
    REQUIRE(security.get_symbol() == "S32");
    REQUIRE(security.get_venue() == ASX);
  }

  TEST_CASE("empty") {
    REQUIRE_FALSE(static_cast<bool>(Security()));
    REQUIRE(static_cast<bool>(Security("S32", ASX)));
  }

  TEST_CASE("hash") {
    auto x = Security("SHOP", TSX);
    auto y = Security("SHOP", TSX);
    auto hasher = std::hash<Security>();
    REQUIRE(hasher(x) == hasher(y));
  }

  TEST_CASE("parse") {
    auto invalid = parse_security("XYZ", DEFAULT_VENUES);
    REQUIRE(!invalid);
    auto valid = parse_security("TD.TSX", DEFAULT_VENUES);
    REQUIRE(valid.get_symbol() == "TD");
    REQUIRE(valid.get_venue() == TSX);
    auto compound = parse_security("A.B.C.TSX", DEFAULT_VENUES);
    REQUIRE(compound.get_symbol() == "A.B.C");
    REQUIRE(compound.get_venue() == TSX);
  }

  TEST_CASE("stream") {
    auto security = Security("ARQ", TSXV);
    REQUIRE(to_string(security) == "ARQ.TSXV");
    auto empty = Security();
    REQUIRE(to_string(empty) == "");
    test_round_trip_shuttle(security);
  }

  TEST_CASE("stream_input") {
    auto ss = std::istringstream("ETHC.NEOE");
    auto parsed_security = Security();
    ss >> parsed_security;
    REQUIRE(parsed_security == Security("ETHC", NEOE));
    ss = std::istringstream("INVALID");
    auto invalid_security = Security();
    ss >> invalid_security;
    REQUIRE(!invalid_security);
  }

  TEST_CASE("parse_security_set") {
    auto yaml = R"(
      - ABX.TSX
      - RY.TSX
      - ETHC.NEOE
    )";
    auto node = YAML::Load(yaml);
    auto securities = parse_security_set(node);
    REQUIRE(securities.size() == 3);
    REQUIRE(securities.contains(Security("ABX", TSX)));
    REQUIRE(securities.contains(Security("RY", TSX)));
    REQUIRE(securities.contains(Security("ETHC", NEOE)));
  }

  TEST_CASE("parse_security_set_with_invalid") {
    auto yaml = R"(
      - ABX.TSX
      - "   .TSX"
      - INVALID
      - RY.TSX
    )";
    auto node = YAML::Load(yaml);
    auto securities = parse_security_set(node);
    REQUIRE(securities.size() == 2);
    REQUIRE(securities.contains(Security("ABX", TSX)));
    REQUIRE(securities.contains(Security("RY", TSX)));
  }
}
