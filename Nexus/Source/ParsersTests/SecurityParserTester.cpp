#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/SecurityParser.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("SecurityParser") {
  TEST_CASE("well_formed_security") {
    auto parser = SecurityParser();
    auto stream = to_parser_stream("ABX.TSX");
    auto security = Security();
    REQUIRE(parser.read(stream, security));
    REQUIRE(security.get_symbol() == "ABX");
    REQUIRE(security.get_venue() == TSX);
  }

  TEST_CASE("invalid_venue") {
    auto parser = SecurityParser();
    auto stream = to_parser_stream("ABX.XST");
    auto security = Security();
    REQUIRE_FALSE(parser.read(stream, security));
  }

  TEST_CASE("lowercase_security") {
    auto parser = SecurityParser();
    auto stream = to_parser_stream("aBx.TsX");
    auto security = Security();
    REQUIRE(parser.read(stream, security));
    REQUIRE(security.get_symbol() == "ABX");
    REQUIRE(security.get_venue() == TSX);
  }

  TEST_CASE("missing_dot") {
    auto parser = SecurityParser();
    auto stream = to_parser_stream("ABXTSX");
    auto security = Security();
    REQUIRE_FALSE(parser.read(stream, security));
  }

  TEST_CASE("empty_symbol") {
    auto parser = SecurityParser();
    auto stream = to_parser_stream(".TSX");
    auto security = Security();
    REQUIRE_FALSE(parser.read(stream, security));
  }

  TEST_CASE("empty_venue") {
    auto parser = SecurityParser();
    auto stream = to_parser_stream("ABX.");
    auto security = Security();
    REQUIRE_FALSE(parser.read(stream, security));
  }
}
