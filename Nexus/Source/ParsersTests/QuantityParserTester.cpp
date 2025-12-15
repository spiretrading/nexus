#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/QuantityParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("QuantityParser") {
  TEST_CASE("valid_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("123.456");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("123.456"));
  }

  TEST_CASE("integer_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("100");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("100"));
  }

  TEST_CASE("negative_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("-42.5");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("-42.5"));
  }

  TEST_CASE("zero_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("0.0");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == Quantity(0));
  }

  TEST_CASE("invalid_quantity") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("abc");
    auto quantity = Quantity();
    REQUIRE_FALSE(parser.read(stream, quantity));
  }

  TEST_CASE("multiple_quantities") {
    auto parser = quantity_parser();
    auto stream = to_parser_stream("1.5-2.5");
    auto quantity = Quantity();
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("1.5"));
    REQUIRE(parser.read(stream, quantity));
    REQUIRE(quantity == parse_quantity("-2.5"));
  }
}
