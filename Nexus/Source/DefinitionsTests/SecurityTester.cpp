#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Security.hpp"

using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("Security") {
  TEST_CASE("default") {
    auto security = Security();
    CHECK(security.get_symbol().empty());
    CHECK(security.get_venue() == Venue());
  }

  TEST_CASE("constructor") {
    auto security = Security("AAPL", NASDAQ);
    CHECK(security.get_symbol() == "AAPL");
    CHECK(security.get_venue() == NASDAQ);
  }

  TEST_CASE("hash") {
    auto x = Security("IBM", NYSE);
    auto y = Security("IBM", NYSE);
    auto hasher = std::hash<Security>();
    CHECK(hasher(x) == hasher(y));
  }

  TEST_CASE("parse") {
    auto invalid = parse_security("XYZ", DEFAULT_VENUES);
    CHECK(invalid == Security());
    auto valid = parse_security("TSLA.NSDQ", DEFAULT_VENUES);
    CHECK(valid.get_symbol() == "TSLA");
    CHECK(valid.get_venue() == NASDAQ);
    auto compound = parse_security("A.B.C.TSX", DEFAULT_VENUES);
    CHECK(compound.get_symbol() == "A.B.C");
    CHECK(compound.get_venue() == TSX);
  }

  TEST_CASE("stream") {
    auto security = Security("IBM", NYSE);
    auto ss = std::ostringstream();
    ss << security;
    CHECK(ss.str() == "IBM.NYSE");
    auto empty = Security();
    ss = std::ostringstream();
    ss << empty;
    CHECK(ss.str() == "");
  }

  TEST_CASE("stream_input") {
    auto ss = std::istringstream("AAPL.NASDAQ");
    auto parsed_security = Security();
    ss >> parsed_security;
    CHECK(parsed_security == Security("AAPL", NASDAQ));
    ss = std::istringstream("INVALID");
    auto invalid_security = Security();
    ss >> invalid_security;
    CHECK(invalid_security == Security());
  }
}
