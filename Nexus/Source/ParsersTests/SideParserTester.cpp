#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/SideParser.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Nexus;

TEST_SUITE("SideParser") {
  TEST_CASE("all_sides") {
    auto parser = side_parser();
    auto cases = std::array{
      std::make_tuple("BID", Side::BID),
      std::make_tuple("ASK", Side::ASK)
    };
    for(auto& entry : cases) {
      auto stream = ParserStreamFromString(std::get<0>(entry));
      auto side = Side();
      REQUIRE(parser.Read(stream, side));
      REQUIRE(side == std::get<1>(entry));
    }
  }

  TEST_CASE("invalid_side") {
    auto parser = side_parser();
    auto stream = ParserStreamFromString("INVALID_SIDE");
    auto side = Side();
    REQUIRE_FALSE(parser.Read(stream, side));
  }

  TEST_CASE("lowercase_side") {
    auto parser = side_parser();
    auto stream = ParserStreamFromString("bid");
    auto side = Side();
    REQUIRE_FALSE(parser.Read(stream, side));
  }

  TEST_CASE("partial_side") {
    auto parser = side_parser();
    auto stream = ParserStreamFromString("BI");
    auto side = Side();
    REQUIRE_FALSE(parser.Read(stream, side));
  }

  TEST_CASE("multiple_sides") {
    auto parser = side_parser();
    auto stream = ParserStreamFromString("BIDASK");
    auto side = Side();
    REQUIRE(parser.Read(stream, side));
    REQUIRE(side == Side::BID);
    REQUIRE(parser.Read(stream, side));
    REQUIRE(side == Side::ASK);
  }
}
