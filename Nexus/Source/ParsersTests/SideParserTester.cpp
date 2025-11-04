#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/SideParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("SideParser") {
  TEST_CASE("all_sides") {
    auto parser = side_parser();
    auto cases = std::array{
      std::make_tuple("BID", Side::BID),
      std::make_tuple("ASK", Side::ASK)
    };
    for(auto& entry : cases) {
      auto stream = to_parser_stream(std::get<0>(entry));
      auto side = Side();
      REQUIRE(parser.read(stream, side));
      REQUIRE(side == std::get<1>(entry));
    }
  }

  TEST_CASE("invalid_side") {
    auto parser = side_parser();
    auto stream = to_parser_stream("INVALID_SIDE");
    auto side = Side();
    REQUIRE_FALSE(parser.read(stream, side));
  }

  TEST_CASE("lowercase_side") {
    auto parser = side_parser();
    auto stream = to_parser_stream("bid");
    auto side = Side();
    REQUIRE_FALSE(parser.read(stream, side));
  }

  TEST_CASE("partial_side") {
    auto parser = side_parser();
    auto stream = to_parser_stream("BI");
    auto side = Side();
    REQUIRE_FALSE(parser.read(stream, side));
  }

  TEST_CASE("multiple_sides") {
    auto parser = side_parser();
    auto stream = to_parser_stream("BIDASK");
    auto side = Side();
    REQUIRE(parser.read(stream, side));
    REQUIRE(side == Side::BID);
    REQUIRE(parser.read(stream, side));
    REQUIRE(side == Side::ASK);
  }
}
