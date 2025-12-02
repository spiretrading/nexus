#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/ExchangeRateTable.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("ExchangeRateTable") {
  TEST_CASE("find") {
    auto table = ExchangeRateTable();
    auto pair = CurrencyPair(USD, EUR);
    auto result = table.find(pair);
    REQUIRE(!result);
  }

  TEST_CASE("add_and_find") {
    auto table = ExchangeRateTable();
    auto pair = CurrencyPair(USD, EUR);
    auto rate_value = rational<int>(3, 2);
    auto rate = ExchangeRate(pair, rate_value);
    table.add(rate);
    auto found = table.find(pair);
    REQUIRE(found);
    REQUIRE(*found == rate);
  }

  TEST_CASE("convert") {
    auto table = ExchangeRateTable();
    auto pair = CurrencyPair(USD, CAD);
    auto rate_value = rational<int>(3, 2);
    table.add(ExchangeRate(pair, rate_value));
    auto amount = Money(100);
    auto converted = table.convert(amount, pair);
    auto expected = Money(150);
    REQUIRE(converted == expected);
  }

  TEST_CASE("convert_base_counter") {
    auto table = ExchangeRateTable();
    auto rate_value = rational<int>(2, 5);
    table.add(ExchangeRate(CurrencyPair(XBT, HKD), rate_value));
    auto amount = Money(100);
    auto converted = table.convert(amount, XBT, HKD);
    auto expected = Money(40);
    REQUIRE(converted == expected);
  }

  TEST_CASE("convert_fail") {
    auto table = ExchangeRateTable();
    auto pair = CurrencyPair(GBP, JPY);
    auto value = Money(1);
    REQUIRE_THROWS_AS(
      table.convert(value, pair), CurrencyPairNotFoundException);
    REQUIRE_THROWS_AS(
      table.convert(value, GBP, JPY), CurrencyPairNotFoundException);
  }

  TEST_CASE("convert_indirect") {
    auto table = ExchangeRateTable();
    table.add(ExchangeRate(CurrencyPair(USD, CAD), rational<int>(2, 1)));
    table.add(ExchangeRate(CurrencyPair(CAD, EUR), rational<int>(4, 1)));
    auto usd_eur = CurrencyPair(USD, EUR);
    auto found = table.find(usd_eur);
    REQUIRE(found);
    REQUIRE(*found == ExchangeRate(usd_eur, rational<int>(8, 1)));
    auto amount = Money(50);
    auto converted = table.convert(amount, USD, EUR);
    REQUIRE(converted == Money(400));
  }

  TEST_CASE("convert_long_chain") {
    auto table = ExchangeRateTable();
    table.add(ExchangeRate(CurrencyPair(USD, CAD), rational<int>(2, 1)));
    table.add(ExchangeRate(CurrencyPair(CAD, EUR), rational<int>(3, 1)));
    table.add(ExchangeRate(CurrencyPair(EUR, GBP), rational<int>(5, 1)));
    auto usd_gbp = CurrencyPair(USD, GBP);
    auto found = table.find(usd_gbp);
    REQUIRE(found);
    REQUIRE(*found == ExchangeRate(usd_gbp, rational<int>(30, 1)));
    auto value = Money(7);
    auto result = table.convert(value, USD, GBP);
    REQUIRE(result == Money(210));
  }

  TEST_CASE("inverse") {
    auto table = ExchangeRateTable();
    auto direct_pair = CurrencyPair(USD, EUR);
    auto direct_rate = rational<int>(3, 2);
    table.add(ExchangeRate(direct_pair, direct_rate));
    auto inverse_pair = CurrencyPair(EUR, USD);
    auto inverse = table.find(inverse_pair);
    REQUIRE(inverse.has_value());
    REQUIRE(*inverse == ExchangeRate(inverse_pair, rational<int>(2, 3)));
    auto euro_amount = Money(150);
    auto converted_usd = table.convert(euro_amount, EUR, USD);
    REQUIRE(converted_usd == Money(100));
  }

  TEST_CASE("identity") {
    auto table = ExchangeRateTable();
    auto converted = table.convert(Money(12), USD, USD);
    REQUIRE(converted == Money(12));
    auto rate = table.find(CurrencyPair(USD, USD));
    REQUIRE(rate == ExchangeRate(CurrencyPair(USD, USD), 1));
  }

  TEST_CASE("shuttle") {
    auto table = ExchangeRateTable();
    table.add(ExchangeRate(CurrencyPair(USD, CAD), rational<int>(2, 1)));
    table.add(ExchangeRate(CurrencyPair(CAD, EUR), rational<int>(3, 1)));
    table.add(ExchangeRate(CurrencyPair(EUR, GBP), rational<int>(5, 1)));
    test_round_trip_shuttle(table, [&] (const auto& received) {
      REQUIRE(received.find(CurrencyPair(USD, CAD)) == 
        ExchangeRate(CurrencyPair(USD, CAD), rational<int>(2, 1)));
      REQUIRE(received.find(CurrencyPair(CAD, EUR)) == 
        ExchangeRate(CurrencyPair(CAD, EUR), rational<int>(3, 1)));
      REQUIRE(received.find(CurrencyPair(EUR, GBP)) == 
        ExchangeRate(CurrencyPair(EUR, GBP), rational<int>(5, 1)));
    });
  }
}
