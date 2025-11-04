#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/MoneyParser.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("MoneyParser") {
  TEST_CASE("valid_money") {
    auto parser = money_parser();
    auto stream = to_parser_stream("123.45");
    auto money = Money();
    REQUIRE(parser.read(stream, money));
    REQUIRE(money == parse_money("123.45"));
  }

  TEST_CASE("integer_money") {
    auto parser = money_parser();
    auto stream = to_parser_stream("100");
    auto money = Money();
    REQUIRE(parser.read(stream, money));
    REQUIRE(money == parse_money("100"));
  }

  TEST_CASE("negative_money") {
    auto parser = money_parser();
    auto stream = to_parser_stream("-42.5");
    auto money = Money();
    REQUIRE(parser.read(stream, money));
    REQUIRE(money == parse_money("-42.5"));
  }

  TEST_CASE("zero_money") {
    auto parser = money_parser();
    auto stream = to_parser_stream("0.00");
    auto money = Money();
    REQUIRE(parser.read(stream, money));
    REQUIRE(money == Money::ZERO);
  }

  TEST_CASE("invalid_money") {
    auto parser = money_parser();
    auto stream = to_parser_stream("abc");
    auto money = Money();
    REQUIRE_FALSE(parser.read(stream, money));
  }

  TEST_CASE("multiple_money") {
    auto parser = money_parser();
    auto stream = to_parser_stream("1.5-2.5");
    auto money = Money();
    REQUIRE(parser.read(stream, money));
    REQUIRE(money == parse_money("1.5"));
    REQUIRE(parser.read(stream, money));
    REQUIRE(money == parse_money("-2.5"));
  }
}
