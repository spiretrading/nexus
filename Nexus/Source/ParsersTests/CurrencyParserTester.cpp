#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Parsers/ReaderParserStream.hpp>
#include <doctest/doctest.h>
#include "Nexus/Parsers/CurrencyParser.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("CurrencyParser") {
  TEST_CASE("valid_currency") {
    auto parser = currency_parser();
    auto stream = ParserStreamFromString("USD");
    auto currency = CurrencyId();
    REQUIRE(parser.Read(stream, currency));
    REQUIRE(currency == USD);
  }

  TEST_CASE("invalid_currency") {
    auto parser = currency_parser();
    auto stream = ParserStreamFromString("ZZZ");
    auto currency = CurrencyId();
    REQUIRE_FALSE(parser.Read(stream, currency));
    REQUIRE(!currency);
  }

  TEST_CASE("lowercase_currency") {
    auto parser = currency_parser();
    auto stream = ParserStreamFromString("usd");
    auto currency = CurrencyId();
    REQUIRE_FALSE(parser.Read(stream, currency));
    REQUIRE(!currency);
  }

  TEST_CASE("partial_currency") {
    auto parser = currency_parser();
    auto stream = ParserStreamFromString("US");
    auto currency = CurrencyId();
    REQUIRE_FALSE(parser.Read(stream, currency));
    REQUIRE(!currency);
  }

  TEST_CASE("multiple_currencies") {
    auto parser = currency_parser();
    auto stream = ParserStreamFromString("USDJPY");
    auto currency = CurrencyId();
    REQUIRE(parser.Read(stream, currency));
    REQUIRE(currency == USD);
    REQUIRE(parser.Read(stream, currency));
    REQUIRE(currency == JPY);
  }
}
