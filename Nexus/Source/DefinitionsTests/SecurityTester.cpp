#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Security.hpp"

using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("Security") {
  TEST_CASE("default") {
    auto security = Security();
    REQUIRE(security.get_symbol().empty());
    REQUIRE(security.get_venue() == Venue());
  }

  TEST_CASE("constructor") {
    auto security = Security("AAPL", NASDAQ);
    REQUIRE(security.get_symbol() == "AAPL");
    REQUIRE(security.get_venue() == NASDAQ);
  }

  TEST_CASE("hash") {
    auto x = Security("IBM", NYSE);
    auto y = Security("IBM", NYSE);
    auto hasher = std::hash<Security>();
    REQUIRE(hasher(x) == hasher(y));
  }

  TEST_CASE("parse") {
    auto invalid = parse_security("XYZ", DEFAULT_VENUES);
    REQUIRE(invalid == Security());
    auto valid = parse_security("TSLA.NSDQ", DEFAULT_VENUES);
    REQUIRE(valid.get_symbol() == "TSLA");
    REQUIRE(valid.get_venue() == NASDAQ);
    auto compound = parse_security("A.B.C.TSX", DEFAULT_VENUES);
    REQUIRE(compound.get_symbol() == "A.B.C");
    REQUIRE(compound.get_venue() == TSX);
  }

  TEST_CASE("stream") {
    auto security = Security("IBM", NYSE);
    auto ss = std::ostringstream();
    ss << security;
    REQUIRE(ss.str() == "IBM.NYSE");
    auto empty = Security();
    ss = std::ostringstream();
    ss << empty;
    REQUIRE(ss.str() == "");
  }

  TEST_CASE("stream_input") {
    auto ss = std::istringstream("AAPL.NSDQ");
    auto parsed_security = Security();
    ss >> parsed_security;
    REQUIRE(parsed_security == Security("AAPL", NASDAQ));
    ss = std::istringstream("INVALID");
    auto invalid_security = Security();
    ss >> invalid_security;
    REQUIRE(invalid_security == Security());
  }
}
