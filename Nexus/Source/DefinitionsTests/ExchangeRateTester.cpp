#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/ExchangeRate.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

TEST_SUITE("ExchangeRate") {
  TEST_CASE("default_constructor") {
    auto rate = ExchangeRate();
    REQUIRE(rate.m_rate.numerator() == 1);
    REQUIRE(rate.m_rate.denominator() == 1);
    REQUIRE(rate.m_pair == CurrencyPair());
  }

  TEST_CASE("constructor") {
    auto pair = CurrencyPair(USD, EUR);
    auto rate  = rational<int>(3, 4);
    auto exchange_rate = ExchangeRate(pair, rate);
    REQUIRE(exchange_rate.m_rate == rate);
    REQUIRE(exchange_rate.m_pair == pair);
  }

  TEST_CASE("invert") {
    auto pair = CurrencyPair(GBP, JPY);
    auto rate  = rational<int>(2, 5);
    auto exchange_rate = ExchangeRate(pair, rate);
    auto inverse_rate = invert(exchange_rate);
    REQUIRE(inverse_rate.m_rate.numerator() == rate.denominator());
    REQUIRE(inverse_rate.m_rate.denominator() == rate.numerator());
    REQUIRE(inverse_rate.m_pair == invert(pair));
  }

  TEST_CASE("invert_twice") {
    auto pair = CurrencyPair(AUD, CAD);
    auto rate  = rational<int>(7, 9);
    auto exchange_rate = ExchangeRate(pair, rate);
    auto back = invert(invert(exchange_rate));
    REQUIRE(back == exchange_rate);
  }

  TEST_CASE("convert") {
    auto rate = rational<int>(3, 2);
    auto original = 100 * Money::ONE;
    auto exchange_rate = ExchangeRate(CurrencyPair(EUR, GBP), rate);
    auto converted = convert(original, exchange_rate);
    auto expected  = (rate.numerator() * original) / rate.denominator();
    REQUIRE(converted == expected);
  }

  TEST_CASE("convert_then_invert") {
    auto rate = rational<int>(5, 3);
    auto original = 45 * Money::ONE;
    auto exchange_rate = ExchangeRate(CurrencyPair(USD, CAD), rate);
    auto converted = convert(original, exchange_rate);
    auto back = convert(converted, invert(exchange_rate));
    REQUIRE(back == original);
  }

  TEST_CASE("stream") {
    auto pair = CurrencyPair(EUR, USD);
    auto rate = ExchangeRate(pair, rational<int>(3, 2));
    CHECK(to_string(rate) == "(EUR/USD 3/2)");
    test_round_trip_shuttle(rate);
  }
}
