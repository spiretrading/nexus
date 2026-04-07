#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/OrderTypeParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OrderTypeParser") {
  TEST_CASE("all_order_types") {
    auto parser = order_type_parser();
    auto cases = std::array{
      std::tuple("MARKET", OrderType::MARKET),
      std::tuple("LIMIT", OrderType::LIMIT),
      std::tuple("PEGGED", OrderType::PEGGED),
      std::tuple("STOP", OrderType::STOP)};
    for(auto& entry : cases) {
      auto stream = to_parser_stream(std::get<0>(entry));
      auto type = OrderType();
      REQUIRE(parser.read(stream, type));
      REQUIRE(type == std::get<1>(entry));
    }
  }

  TEST_CASE("invalid_order_type") {
    auto parser = order_type_parser();
    auto stream = to_parser_stream("INVALID_TYPE");
    auto type = OrderType();
    REQUIRE_FALSE(parser.read(stream, type));
  }

  TEST_CASE("lowercase_order_type") {
    auto parser = order_type_parser();
    auto stream = to_parser_stream("market");
    auto type = OrderType();
    REQUIRE_FALSE(parser.read(stream, type));
  }

  TEST_CASE("partial_order_type") {
    auto parser = order_type_parser();
    auto stream = to_parser_stream("MAR");
    auto type = OrderType();
    REQUIRE_FALSE(parser.read(stream, type));
  }

  TEST_CASE("multiple_order_types") {
    auto parser = order_type_parser();
    auto stream = to_parser_stream("MARKETLIMIT");
    auto type = OrderType();
    REQUIRE(parser.read(stream, type));
    REQUIRE(type == OrderType::MARKET);
    REQUIRE(parser.read(stream, type));
    REQUIRE(type == OrderType::LIMIT);
  }
}
