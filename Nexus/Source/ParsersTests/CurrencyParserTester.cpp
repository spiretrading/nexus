#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/CurrencyParser.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("CurrencyParser") {
  TEST_CASE("valid_currency") {
    auto parser = currency_parser();
    auto stream = to_parser_stream("USD");
    auto currency = CurrencyId();
    REQUIRE(parser.read(stream, currency));
    REQUIRE(currency == USD);
  }

  TEST_CASE("invalid_currency") {
    auto parser = currency_parser();
    auto stream = to_parser_stream("ZZZ");
    auto currency = CurrencyId();
    REQUIRE_FALSE(parser.read(stream, currency));
    REQUIRE(!currency);
  }

  TEST_CASE("lowercase_currency") {
    auto parser = currency_parser();
    auto stream = to_parser_stream("usd");
    auto currency = CurrencyId();
    REQUIRE_FALSE(parser.read(stream, currency));
    REQUIRE(!currency);
  }

  TEST_CASE("partial_currency") {
    auto parser = currency_parser();
    auto stream = to_parser_stream("US");
    auto currency = CurrencyId();
    REQUIRE_FALSE(parser.read(stream, currency));
    REQUIRE(!currency);
  }

  TEST_CASE("multiple_currencies") {
    auto parser = currency_parser();
    auto stream = to_parser_stream("USDJPY");
    auto currency = CurrencyId();
    REQUIRE(parser.read(stream, currency));
    REQUIRE(currency == USD);
    REQUIRE(parser.read(stream, currency));
    REQUIRE(currency == JPY);
  }
}
