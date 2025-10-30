#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/CurrencyPair.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("CurrencyPair") {
  TEST_CASE("parse_currency_pair") {
    auto pair = parse_currency_pair("JPY/AUD");
    REQUIRE(
      pair == CurrencyPair(DefaultCurrencies::JPY, DefaultCurrencies::AUD));
  }

  TEST_CASE("parse_currency_pair_no_separator") {
    REQUIRE_THROWS_AS(parse_currency_pair("CADUSD"), std::runtime_error);
  }

  TEST_CASE("parse_currency_pair_no_base") {
    REQUIRE_THROWS_AS(parse_currency_pair("XXX/USD"), std::runtime_error);
  }

  TEST_CASE("parse_currency_pair_no_counter") {
    REQUIRE_THROWS_AS(parse_currency_pair("CAD/YYY"), std::runtime_error);
  }

  TEST_CASE("invert") {
    auto original = CurrencyPair(CurrencyId(1), CurrencyId(2));
    auto inverted = invert(original);
    REQUIRE(inverted == CurrencyPair(CurrencyId(2), CurrencyId(1)));
  }

  TEST_CASE("stream") {
    auto pair = CurrencyPair(DefaultCurrencies::CAD, DefaultCurrencies::USD);
    REQUIRE(to_string(pair) == "CAD/USD");
    test_round_trip_shuttle(pair);
  }
}
