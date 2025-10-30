#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Quantity.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;

TEST_SUITE("Quantity") {
  TEST_CASE("to_string") {
    REQUIRE(to_string(Quantity(0)) == "0");
    REQUIRE(to_string(Quantity(1)) == "1");
    REQUIRE(to_string(Quantity(1.1)) == "1.100000");
  }

  TEST_CASE("from_string") {
    REQUIRE(parse_quantity("1") == Quantity(1));
    REQUIRE(parse_quantity("1.1") == Quantity(1.1));
    REQUIRE(try_parse_quantity("1a.1") == none);
  }

  TEST_CASE("assignment") {
    auto q = Quantity(0);
    q = 100;
    REQUIRE(q == 100);
  }

  TEST_CASE("modulus") {
    REQUIRE(Quantity(1) % Quantity(5) == Quantity(1));
    REQUIRE(Quantity(3) % Quantity(2) == Quantity(1));
  }

  TEST_CASE("ceil") {
    SUBCASE("positive_integers") {
      REQUIRE(ceil(parse_quantity("0")) == parse_quantity("0"));
      REQUIRE(ceil(parse_quantity("1")) == parse_quantity("1"));
      REQUIRE(ceil(parse_quantity("5")) == parse_quantity("5"));
      REQUIRE(ceil(parse_quantity("100")) == parse_quantity("100"));
    }

    SUBCASE("positive_decimals") {
      REQUIRE(ceil(parse_quantity("0.1")) == parse_quantity("1"));
      REQUIRE(ceil(parse_quantity("0.5")) == parse_quantity("1"));
      REQUIRE(ceil(parse_quantity("0.9")) == parse_quantity("1"));
      REQUIRE(ceil(parse_quantity("1.1")) == parse_quantity("2"));
      REQUIRE(ceil(parse_quantity("1.5")) == parse_quantity("2"));
      REQUIRE(ceil(parse_quantity("1.9")) == parse_quantity("2"));
      REQUIRE(ceil(parse_quantity("5.001")) == parse_quantity("6"));
      REQUIRE(ceil(parse_quantity("99.999999")) == parse_quantity("100"));
    }

    SUBCASE("negative_integers") {
      REQUIRE(ceil(parse_quantity("-1")) == parse_quantity("-1"));
      REQUIRE(ceil(parse_quantity("-5")) == parse_quantity("-5"));
      REQUIRE(ceil(parse_quantity("-100")) == parse_quantity("-100"));
    }

    SUBCASE("negative_decimals") {
      REQUIRE(ceil(parse_quantity("-0.1")) == parse_quantity("0"));
      REQUIRE(ceil(parse_quantity("-0.5")) == parse_quantity("0"));
      REQUIRE(ceil(parse_quantity("-0.9")) == parse_quantity("0"));
      REQUIRE(ceil(parse_quantity("-1.1")) == parse_quantity("-1"));
      REQUIRE(ceil(parse_quantity("-1.5")) == parse_quantity("-1"));
      REQUIRE(ceil(parse_quantity("-1.9")) == parse_quantity("-1"));
      REQUIRE(ceil(parse_quantity("-5.001")) == parse_quantity("-5"));
      REQUIRE(ceil(parse_quantity("-99.999999")) == parse_quantity("-99"));
    }

    SUBCASE("very_small_values") {
      REQUIRE(ceil(parse_quantity("0.000001")) == parse_quantity("1"));
      REQUIRE(ceil(parse_quantity("-0.000001")) == parse_quantity("0"));
      REQUIRE(ceil(parse_quantity("0.0000001")) == parse_quantity("1"));
    }

    SUBCASE("large_values") {
      REQUIRE(ceil(parse_quantity("1000000.1")) == parse_quantity("1000001"));
      REQUIRE(ceil(parse_quantity("-1000000.1")) == parse_quantity("-1000000"));
    }
  }

  TEST_CASE("ceil_to") {
    SUBCASE("zero_multiple") {
      REQUIRE(ceil_to(parse_quantity("5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("-5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
    }

    SUBCASE("multiple_of_one") {
      REQUIRE(ceil_to(parse_quantity("0"), parse_quantity("1")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(ceil_to(parse_quantity("1.1"), parse_quantity("1")) ==
        parse_quantity("2"));
      REQUIRE(ceil_to(parse_quantity("1.9"), parse_quantity("1")) ==
        parse_quantity("2"));
      REQUIRE(ceil_to(parse_quantity("-1.1"), parse_quantity("1")) ==
        parse_quantity("-1"));
      REQUIRE(ceil_to(parse_quantity("-1.9"), parse_quantity("1")) ==
        parse_quantity("-1"));
    }

    SUBCASE("positive_value_positive_multiple") {
      REQUIRE(ceil_to(parse_quantity("0"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("1"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(ceil_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(ceil_to(parse_quantity("6"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(ceil_to(parse_quantity("9.9"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(ceil_to(parse_quantity("10"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(ceil_to(parse_quantity("10.1"), parse_quantity("5")) ==
        parse_quantity("15"));
    }

    SUBCASE("negative_value_positive_multiple") {
      REQUIRE(ceil_to(parse_quantity("-1"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(ceil_to(parse_quantity("-6"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(ceil_to(parse_quantity("-9.9"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(ceil_to(parse_quantity("-10"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(ceil_to(parse_quantity("-10.1"), parse_quantity("5")) ==
        parse_quantity("-10"));
    }

    SUBCASE("fractional_multiples") {
      REQUIRE(ceil_to(parse_quantity("0"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("0.1"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(ceil_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(ceil_to(parse_quantity("0.6"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(ceil_to(parse_quantity("1"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(ceil_to(parse_quantity("1.1"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
      REQUIRE(ceil_to(parse_quantity("1.5"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
      REQUIRE(ceil_to(parse_quantity("1.6"), parse_quantity("0.5")) ==
        parse_quantity("2"));
    }

    SUBCASE("negative_fractional_multiples") {
      REQUIRE(ceil_to(parse_quantity("-0.1"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("-0.5"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(ceil_to(parse_quantity("-0.6"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(ceil_to(parse_quantity("-1"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(ceil_to(parse_quantity("-1.1"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(ceil_to(parse_quantity("-1.5"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
      REQUIRE(ceil_to(parse_quantity("-1.6"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
    }

    SUBCASE("tick_size_examples") {
      REQUIRE(ceil_to(parse_quantity("100.12"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(ceil_to(parse_quantity("100.25"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(ceil_to(parse_quantity("100.26"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(ceil_to(parse_quantity("100.51"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
      REQUIRE(ceil_to(parse_quantity("100.76"), parse_quantity("0.25")) ==
        parse_quantity("101"));
    }

    SUBCASE("decimal_multiples") {
      REQUIRE(ceil_to(parse_quantity("1.234"), parse_quantity("0.1")) ==
        parse_quantity("1.3"));
      REQUIRE(ceil_to(parse_quantity("1.234"), parse_quantity("0.01")) ==
        parse_quantity("1.24"));
      REQUIRE(ceil_to(parse_quantity("1.234"), parse_quantity("0.001")) ==
        parse_quantity("1.234"));
    }

    SUBCASE("large_multiples") {
      REQUIRE(ceil_to(parse_quantity("123"), parse_quantity("100")) ==
        parse_quantity("200"));
      REQUIRE(ceil_to(parse_quantity("100"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(ceil_to(parse_quantity("99"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(ceil_to(parse_quantity("1"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(ceil_to(parse_quantity("-1"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("-99"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("-100"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(ceil_to(parse_quantity("-123"), parse_quantity("100")) ==
        parse_quantity("-100"));
    }

    SUBCASE("very_small_multiples") {
      REQUIRE(ceil_to(
        parse_quantity("1.123456"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
      REQUIRE(ceil_to(
        parse_quantity("1.1234567"), parse_quantity("0.000001")) ==
          parse_quantity("1.123457"));
    }

    SUBCASE("same_value_and_multiple") {
      REQUIRE(ceil_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(ceil_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(ceil_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
    }

    SUBCASE("value_less_than_multiple") {
      REQUIRE(ceil_to(parse_quantity("0.1"), parse_quantity("10")) ==
        parse_quantity("10"));
      REQUIRE(ceil_to(parse_quantity("9.99"), parse_quantity("10")) ==
        parse_quantity("10"));
      REQUIRE(ceil_to(parse_quantity("-0.1"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(ceil_to(parse_quantity("-9.99"), parse_quantity("10")) ==
        parse_quantity("0"));
    }

    SUBCASE("precision_edge_cases") {
      auto tiny = parse_quantity("0.000001");
      REQUIRE(ceil_to(parse_quantity("1") + tiny, parse_quantity("1")) ==
        parse_quantity("2"));
      REQUIRE(ceil_to(parse_quantity("1") - tiny, parse_quantity("1")) ==
        parse_quantity("1"));
    }
  }

  TEST_CASE("floor") {
    SUBCASE("positive_integers") {
      REQUIRE(floor(parse_quantity("0")) == parse_quantity("0"));
      REQUIRE(floor(parse_quantity("1")) == parse_quantity("1"));
      REQUIRE(floor(parse_quantity("5")) == parse_quantity("5"));
      REQUIRE(floor(parse_quantity("100")) == parse_quantity("100"));
    }

    SUBCASE("positive_decimals") {
      REQUIRE(floor(parse_quantity("0.1")) == parse_quantity("0"));
      REQUIRE(floor(parse_quantity("0.5")) == parse_quantity("0"));
      REQUIRE(floor(parse_quantity("0.9")) == parse_quantity("0"));
      REQUIRE(floor(parse_quantity("1.1")) == parse_quantity("1"));
      REQUIRE(floor(parse_quantity("1.5")) == parse_quantity("1"));
      REQUIRE(floor(parse_quantity("1.9")) == parse_quantity("1"));
      REQUIRE(floor(parse_quantity("5.001")) == parse_quantity("5"));
      REQUIRE(floor(parse_quantity("99.999999")) == parse_quantity("99"));
    }

    SUBCASE("negative_integers") {
      REQUIRE(floor(parse_quantity("-1")) == parse_quantity("-1"));
      REQUIRE(floor(parse_quantity("-5")) == parse_quantity("-5"));
      REQUIRE(floor(parse_quantity("-100")) == parse_quantity("-100"));
    }

    SUBCASE("negative_decimals") {
      REQUIRE(floor(parse_quantity("-0.1")) == parse_quantity("-1"));
      REQUIRE(floor(parse_quantity("-0.5")) == parse_quantity("-1"));
      REQUIRE(floor(parse_quantity("-0.9")) == parse_quantity("-1"));
      REQUIRE(floor(parse_quantity("-1.1")) == parse_quantity("-2"));
      REQUIRE(floor(parse_quantity("-1.5")) == parse_quantity("-2"));
      REQUIRE(floor(parse_quantity("-1.9")) == parse_quantity("-2"));
      REQUIRE(floor(parse_quantity("-5.001")) == parse_quantity("-6"));
      REQUIRE(floor(parse_quantity("-99.999999")) == parse_quantity("-100"));
    }

    SUBCASE("very_small_values") {
      REQUIRE(floor(parse_quantity("0.000001")) == parse_quantity("0"));
      REQUIRE(floor(parse_quantity("-0.000001")) == parse_quantity("-1"));
      REQUIRE(floor(parse_quantity("0.0000001")) == parse_quantity("0"));
    }

    SUBCASE("large_values") {
      REQUIRE(floor(parse_quantity("1000000.1")) == parse_quantity("1000000"));
      REQUIRE(floor(parse_quantity("-1000000.1")) == 
        parse_quantity("-1000001"));
    }
  }

  TEST_CASE("floor_to") {
    SUBCASE("zero_multiple") {
      REQUIRE(floor_to(parse_quantity("5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("-5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
    }

    SUBCASE("multiple_of_one") {
      REQUIRE(floor_to(parse_quantity("0"), parse_quantity("1")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(floor_to(parse_quantity("1.1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(floor_to(parse_quantity("1.9"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(floor_to(parse_quantity("-1.1"), parse_quantity("1")) ==
        parse_quantity("-2"));
      REQUIRE(floor_to(parse_quantity("-1.9"), parse_quantity("1")) ==
        parse_quantity("-2"));
    }

    SUBCASE("positive_value_positive_multiple") {
      REQUIRE(floor_to(parse_quantity("0"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("1"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(floor_to(parse_quantity("6"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(floor_to(parse_quantity("9.9"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(floor_to(parse_quantity("10"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(floor_to(parse_quantity("10.1"), parse_quantity("5")) ==
        parse_quantity("10"));
    }

    SUBCASE("negative_value_positive_multiple") {
      REQUIRE(floor_to(parse_quantity("-1"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(floor_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(floor_to(parse_quantity("-6"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(floor_to(parse_quantity("-9.9"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(floor_to(parse_quantity("-10"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(floor_to(parse_quantity("-10.1"), parse_quantity("5")) ==
        parse_quantity("-15"));
    }

    SUBCASE("fractional_multiples") {
      REQUIRE(floor_to(parse_quantity("0"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("0.1"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(floor_to(parse_quantity("0.6"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(floor_to(parse_quantity("1"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(floor_to(parse_quantity("1.1"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(floor_to(parse_quantity("1.5"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
      REQUIRE(floor_to(parse_quantity("1.6"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
    }

    SUBCASE("negative_fractional_multiples") {
      REQUIRE(floor_to(parse_quantity("-0.1"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(floor_to(parse_quantity("-0.5"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(floor_to(parse_quantity("-0.6"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(floor_to(parse_quantity("-1"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(floor_to(parse_quantity("-1.1"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
      REQUIRE(floor_to(parse_quantity("-1.5"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
      REQUIRE(floor_to(parse_quantity("-1.6"), parse_quantity("0.5")) ==
        parse_quantity("-2"));
    }

    SUBCASE("tick_size_examples") {
      REQUIRE(floor_to(parse_quantity("100.12"), parse_quantity("0.25")) ==
        parse_quantity("100"));
      REQUIRE(floor_to(parse_quantity("100.25"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(floor_to(parse_quantity("100.26"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(floor_to(parse_quantity("100.51"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(floor_to(parse_quantity("100.76"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
    }

    SUBCASE("decimal_multiples") {
      REQUIRE(floor_to(parse_quantity("1.234"), parse_quantity("0.1")) ==
        parse_quantity("1.2"));
      REQUIRE(floor_to(parse_quantity("1.234"), parse_quantity("0.01")) ==
        parse_quantity("1.23"));
      REQUIRE(floor_to(parse_quantity("1.234"), parse_quantity("0.001")) ==
        parse_quantity("1.234"));
    }

    SUBCASE("large_multiples") {
      REQUIRE(floor_to(parse_quantity("123"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(floor_to(parse_quantity("100"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(floor_to(parse_quantity("99"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("1"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("-1"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(floor_to(parse_quantity("-99"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(floor_to(parse_quantity("-100"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(floor_to(parse_quantity("-123"), parse_quantity("100")) ==
        parse_quantity("-200"));
    }

    SUBCASE("very_small_multiples") {
      REQUIRE(floor_to(
        parse_quantity("1.123456"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
      REQUIRE(floor_to(
        parse_quantity("1.1234567"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
    }

    SUBCASE("same_value_and_multiple") {
      REQUIRE(floor_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(floor_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(floor_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
    }

    SUBCASE("value_less_than_multiple") {
      REQUIRE(floor_to(parse_quantity("0.1"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("9.99"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(floor_to(parse_quantity("-0.1"), parse_quantity("10")) ==
        parse_quantity("-10"));
      REQUIRE(floor_to(parse_quantity("-9.99"), parse_quantity("10")) ==
        parse_quantity("-10"));
    }

    SUBCASE("precision_edge_cases") {
      auto tiny = parse_quantity("0.000001");
      REQUIRE(floor_to(parse_quantity("1") + tiny, parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(floor_to(parse_quantity("1") - tiny, parse_quantity("1")) ==
        parse_quantity("0"));
    }
  }

  TEST_CASE("round") {
    SUBCASE("positive_integers") {
      REQUIRE(round(parse_quantity("0")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("1")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("5")) == parse_quantity("5"));
      REQUIRE(round(parse_quantity("100")) == parse_quantity("100"));
    }

    SUBCASE("positive_decimals") {
      REQUIRE(round(parse_quantity("0.1")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("0.4")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("0.5")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("0.6")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("0.9")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("1.1")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("1.4")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("1.5")) == parse_quantity("2"));
      REQUIRE(round(parse_quantity("1.6")) == parse_quantity("2"));
      REQUIRE(round(parse_quantity("1.9")) == parse_quantity("2"));
      REQUIRE(round(parse_quantity("5.001")) == parse_quantity("5"));
      REQUIRE(round(parse_quantity("5.5")) == parse_quantity("6"));
      REQUIRE(round(parse_quantity("99.4")) == parse_quantity("99"));
      REQUIRE(round(parse_quantity("99.5")) == parse_quantity("100"));
      REQUIRE(round(parse_quantity("99.999999")) == parse_quantity("100"));
    }

    SUBCASE("negative_integers") {
      REQUIRE(round(parse_quantity("-1")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-5")) == parse_quantity("-5"));
      REQUIRE(round(parse_quantity("-100")) == parse_quantity("-100"));
    }

    SUBCASE("negative_decimals") {
      REQUIRE(round(parse_quantity("-0.1")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("-0.4")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("-0.5")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-0.6")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-0.9")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-1.1")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-1.4")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-1.5")) == parse_quantity("-2"));
      REQUIRE(round(parse_quantity("-1.6")) == parse_quantity("-2"));
      REQUIRE(round(parse_quantity("-1.9")) == parse_quantity("-2"));
      REQUIRE(round(parse_quantity("-5.001")) == parse_quantity("-5"));
      REQUIRE(round(parse_quantity("-5.5")) == parse_quantity("-6"));
      REQUIRE(round(parse_quantity("-99.4")) == parse_quantity("-99"));
      REQUIRE(round(parse_quantity("-99.5")) == parse_quantity("-100"));
      REQUIRE(round(parse_quantity("-99.999999")) == parse_quantity("-100"));
    }

    SUBCASE("very_small_values") {
      REQUIRE(round(parse_quantity("0.000001")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("-0.000001")) == parse_quantity("0"));
      REQUIRE(round(parse_quantity("0.0000001")) == parse_quantity("0"));
    }

    SUBCASE("large_values") {
      REQUIRE(round(parse_quantity("1000000.1")) == parse_quantity("1000000"));
      REQUIRE(round(parse_quantity("1000000.5")) == parse_quantity("1000001"));
      REQUIRE(round(parse_quantity("-1000000.1")) ==
        parse_quantity("-1000000"));
      REQUIRE(round(parse_quantity("-1000000.5")) == 
        parse_quantity("-1000001"));
    }

    SUBCASE("half_values") {
      REQUIRE(round(parse_quantity("0.5")) == parse_quantity("1"));
      REQUIRE(round(parse_quantity("1.5")) == parse_quantity("2"));
      REQUIRE(round(parse_quantity("2.5")) == parse_quantity("3"));
      REQUIRE(round(parse_quantity("3.5")) == parse_quantity("4"));
      REQUIRE(round(parse_quantity("-0.5")) == parse_quantity("-1"));
      REQUIRE(round(parse_quantity("-1.5")) == parse_quantity("-2"));
      REQUIRE(round(parse_quantity("-2.5")) == parse_quantity("-3"));
      REQUIRE(round(parse_quantity("-3.5")) == parse_quantity("-4"));
    }
  }

  TEST_CASE("round_to") {
    SUBCASE("zero_multiple") {
      REQUIRE(round_to(parse_quantity("5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
    }

    SUBCASE("multiple_of_one") {
      REQUIRE(round_to(parse_quantity("0"), parse_quantity("1")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1.1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1.5"), parse_quantity("1")) ==
        parse_quantity("2"));
      REQUIRE(round_to(parse_quantity("1.9"), parse_quantity("1")) ==
        parse_quantity("2"));
      REQUIRE(round_to(parse_quantity("-1.1"), parse_quantity("1")) ==
        parse_quantity("-1"));
      REQUIRE(round_to(parse_quantity("-1.5"), parse_quantity("1")) ==
        parse_quantity("-2"));
      REQUIRE(round_to(parse_quantity("-1.9"), parse_quantity("1")) ==
        parse_quantity("-2"));
    }

    SUBCASE("positive_value_positive_multiple") {
      REQUIRE(round_to(parse_quantity("0"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("1"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("2.4"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("2.5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("2.6"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("6"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("7.4"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("7.5"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(round_to(parse_quantity("7.6"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(round_to(parse_quantity("10"), parse_quantity("5")) ==
        parse_quantity("10"));
    }

    SUBCASE("negative_value_positive_multiple") {
      REQUIRE(round_to(parse_quantity("-1"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-2.4"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-2.5"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(round_to(parse_quantity("-2.6"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(round_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(round_to(parse_quantity("-6"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(round_to(parse_quantity("-7.4"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(round_to(parse_quantity("-7.5"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(round_to(parse_quantity("-7.6"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(round_to(parse_quantity("-10"), parse_quantity("5")) ==
        parse_quantity("-10"));
    }

    SUBCASE("fractional_multiples") {
      REQUIRE(round_to(parse_quantity("0"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("0.1"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("0.24"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("0.25"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(round_to(parse_quantity("0.26"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(round_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(round_to(parse_quantity("0.74"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(round_to(parse_quantity("0.75"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("0.76"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1.24"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1.25"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
      REQUIRE(round_to(parse_quantity("1.5"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
    }

    SUBCASE("negative_fractional_multiples") {
      REQUIRE(round_to(parse_quantity("-0.1"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-0.24"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-0.25"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(round_to(parse_quantity("-0.26"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(round_to(parse_quantity("-0.5"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(round_to(parse_quantity("-0.74"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(round_to(parse_quantity("-0.75"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(round_to(parse_quantity("-0.76"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(round_to(parse_quantity("-1"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(round_to(parse_quantity("-1.24"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(round_to(parse_quantity("-1.25"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
      REQUIRE(round_to(parse_quantity("-1.5"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
    }

    SUBCASE("tick_size_examples") {
      REQUIRE(round_to(parse_quantity("100.12"), parse_quantity("0.25")) ==
        parse_quantity("100"));
      REQUIRE(round_to(parse_quantity("100.125"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(round_to(parse_quantity("100.13"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(round_to(parse_quantity("100.25"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(round_to(parse_quantity("100.37"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(round_to(parse_quantity("100.375"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(round_to(parse_quantity("100.38"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(round_to(parse_quantity("100.5"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(round_to(parse_quantity("100.62"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(round_to(parse_quantity("100.625"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
      REQUIRE(round_to(parse_quantity("100.63"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
      REQUIRE(round_to(parse_quantity("100.75"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
      REQUIRE(round_to(parse_quantity("100.87"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
      REQUIRE(round_to(parse_quantity("100.875"), parse_quantity("0.25")) ==
        parse_quantity("101"));
      REQUIRE(round_to(parse_quantity("100.88"), parse_quantity("0.25")) ==
        parse_quantity("101"));
    }

    SUBCASE("decimal_multiples") {
      REQUIRE(round_to(parse_quantity("1.234"), parse_quantity("0.1")) ==
        parse_quantity("1.2"));
      REQUIRE(round_to(parse_quantity("1.249"), parse_quantity("0.1")) ==
        parse_quantity("1.2"));
      REQUIRE(round_to(parse_quantity("1.25"), parse_quantity("0.1")) ==
        parse_quantity("1.3"));
      REQUIRE(round_to(parse_quantity("1.234"), parse_quantity("0.01")) ==
        parse_quantity("1.23"));
      REQUIRE(round_to(parse_quantity("1.2349"), parse_quantity("0.01")) ==
        parse_quantity("1.23"));
      REQUIRE(round_to(parse_quantity("1.235"), parse_quantity("0.01")) ==
        parse_quantity("1.24"));
      REQUIRE(round_to(parse_quantity("1.234"), parse_quantity("0.001")) ==
        parse_quantity("1.234"));
    }

    SUBCASE("large_multiples") {
      REQUIRE(round_to(parse_quantity("49"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("50"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(round_to(parse_quantity("99"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(round_to(parse_quantity("100"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(round_to(parse_quantity("123"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(round_to(parse_quantity("149"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(round_to(parse_quantity("150"), parse_quantity("100")) ==
        parse_quantity("200"));
      REQUIRE(round_to(parse_quantity("-49"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-50"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(round_to(parse_quantity("-99"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(round_to(parse_quantity("-100"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(round_to(parse_quantity("-123"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(round_to(parse_quantity("-149"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(round_to(parse_quantity("-150"), parse_quantity("100")) ==
        parse_quantity("-200"));
    }

    SUBCASE("very_small_multiples") {
      REQUIRE(round_to(
        parse_quantity("1.123456"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
      REQUIRE(round_to(
        parse_quantity("1.1234564"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
      REQUIRE(round_to(
        parse_quantity("1.1234565"), parse_quantity("0.000001")) ==
          parse_quantity("1.123457"));
      REQUIRE(round_to(
        parse_quantity("1.1234567"), parse_quantity("0.000001")) ==
          parse_quantity("1.123457"));
    }

    SUBCASE("same_value_and_multiple") {
      REQUIRE(round_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(round_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
    }

    SUBCASE("value_less_than_multiple") {
      REQUIRE(round_to(parse_quantity("0.1"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("4.9"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("5"), parse_quantity("10")) ==
        parse_quantity("10"));
      REQUIRE(round_to(parse_quantity("9.99"), parse_quantity("10")) ==
        parse_quantity("10"));
      REQUIRE(round_to(parse_quantity("-0.1"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-4.9"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(round_to(parse_quantity("-5"), parse_quantity("10")) ==
        parse_quantity("-10"));
      REQUIRE(round_to(parse_quantity("-9.99"), parse_quantity("10")) ==
        parse_quantity("-10"));
    }

    SUBCASE("precision_edge_cases") {
      auto tiny = parse_quantity("0.000001");
      REQUIRE(round_to(parse_quantity("1") + tiny, parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1") - tiny, parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("0.5") + tiny, parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("0.5") - tiny, parse_quantity("1")) ==
        parse_quantity("0"));
    }

    SUBCASE("half_rounding_behavior") {
      REQUIRE(round_to(parse_quantity("0.5"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(round_to(parse_quantity("1.5"), parse_quantity("1")) ==
        parse_quantity("2"));
      REQUIRE(round_to(parse_quantity("2.5"), parse_quantity("1")) ==
        parse_quantity("3"));
      REQUIRE(round_to(parse_quantity("3.5"), parse_quantity("1")) ==
        parse_quantity("4"));
      REQUIRE(round_to(parse_quantity("4.5"), parse_quantity("1")) ==
        parse_quantity("5"));
      REQUIRE(round_to(parse_quantity("5.5"), parse_quantity("1")) ==
        parse_quantity("6"));
    }
  }

  TEST_CASE("truncate") {
    SUBCASE("positive_integers") {
      REQUIRE(truncate(parse_quantity("0")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("1")) == parse_quantity("1"));
      REQUIRE(truncate(parse_quantity("5")) == parse_quantity("5"));
      REQUIRE(truncate(parse_quantity("100")) == parse_quantity("100"));
    }

    SUBCASE("positive_decimals") {
      REQUIRE(truncate(parse_quantity("0.1")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("0.5")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("0.9")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("1.1")) == parse_quantity("1"));
      REQUIRE(truncate(parse_quantity("1.5")) == parse_quantity("1"));
      REQUIRE(truncate(parse_quantity("1.9")) == parse_quantity("1"));
      REQUIRE(truncate(parse_quantity("5.001")) == parse_quantity("5"));
      REQUIRE(truncate(parse_quantity("99.999999")) == parse_quantity("99"));
    }

    SUBCASE("negative_integers") {
      REQUIRE(truncate(parse_quantity("-1")) == parse_quantity("-1"));
      REQUIRE(truncate(parse_quantity("-5")) == parse_quantity("-5"));
      REQUIRE(truncate(parse_quantity("-100")) == parse_quantity("-100"));
    }

    SUBCASE("negative_decimals") {
      REQUIRE(truncate(parse_quantity("-0.1")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("-0.5")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("-0.9")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("-1.1")) == parse_quantity("-1"));
      REQUIRE(truncate(parse_quantity("-1.5")) == parse_quantity("-1"));
      REQUIRE(truncate(parse_quantity("-1.9")) == parse_quantity("-1"));
      REQUIRE(truncate(parse_quantity("-5.001")) == parse_quantity("-5"));
      REQUIRE(truncate(parse_quantity("-99.999999")) == parse_quantity("-99"));
    }

    SUBCASE("very_small_values") {
      REQUIRE(truncate(parse_quantity("0.000001")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("-0.000001")) == parse_quantity("0"));
      REQUIRE(truncate(parse_quantity("0.0000001")) == parse_quantity("0"));
    }

    SUBCASE("large_values") {
      REQUIRE(truncate(parse_quantity("1000000.1")) == 
        parse_quantity("1000000"));
      REQUIRE(truncate(parse_quantity("1000000.9")) == 
        parse_quantity("1000000"));
      REQUIRE(truncate(parse_quantity("-1000000.1")) == 
        parse_quantity("-1000000"));
      REQUIRE(truncate(parse_quantity("-1000000.9")) == 
        parse_quantity("-1000000"));
    }

    SUBCASE("comparison_with_floor_and_ceil") {
      REQUIRE(truncate(parse_quantity("1.9")) == floor(parse_quantity("1.9")));
      REQUIRE(truncate(parse_quantity("-1.9")) == 
        ceil(parse_quantity("-1.9")));
      REQUIRE(truncate(parse_quantity("5.5")) == floor(parse_quantity("5.5")));
      REQUIRE(truncate(parse_quantity("-5.5")) == 
        ceil(parse_quantity("-5.5")));
    }
  }

  TEST_CASE("truncate_to") {
    SUBCASE("zero_multiple") {
      REQUIRE(truncate_to(parse_quantity("5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-5.5"), parse_quantity("0")) ==
        parse_quantity("0"));
    }

    SUBCASE("multiple_of_one") {
      REQUIRE(truncate_to(parse_quantity("0"), parse_quantity("1")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("1.1"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("1.9"), parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("-1.1"), parse_quantity("1")) ==
        parse_quantity("-1"));
      REQUIRE(truncate_to(parse_quantity("-1.9"), parse_quantity("1")) ==
        parse_quantity("-1"));
    }

    SUBCASE("positive_value_positive_multiple") {
      REQUIRE(truncate_to(parse_quantity("0"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("1"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("4.9"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(truncate_to(parse_quantity("6"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(truncate_to(parse_quantity("9.9"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(truncate_to(parse_quantity("10"), parse_quantity("5")) ==
        parse_quantity("10"));
      REQUIRE(truncate_to(parse_quantity("10.1"), parse_quantity("5")) ==
        parse_quantity("10"));
    }

    SUBCASE("negative_value_positive_multiple") {
      REQUIRE(truncate_to(parse_quantity("-1"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-4.9"), parse_quantity("5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(truncate_to(parse_quantity("-6"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(truncate_to(parse_quantity("-9.9"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(truncate_to(parse_quantity("-10"), parse_quantity("5")) ==
        parse_quantity("-10"));
      REQUIRE(truncate_to(parse_quantity("-10.1"), parse_quantity("5")) ==
        parse_quantity("-10"));
    }

    SUBCASE("fractional_multiples") {
      REQUIRE(truncate_to(parse_quantity("0"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("0.1"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("0.4"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(truncate_to(parse_quantity("0.6"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(truncate_to(parse_quantity("1"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("1.1"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("1.4"), parse_quantity("0.5")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("1.5"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
      REQUIRE(truncate_to(parse_quantity("1.9"), parse_quantity("0.5")) ==
        parse_quantity("1.5"));
    }

    SUBCASE("negative_fractional_multiples") {
      REQUIRE(truncate_to(parse_quantity("-0.1"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-0.4"), parse_quantity("0.5")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-0.5"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(truncate_to(parse_quantity("-0.6"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
      REQUIRE(truncate_to(parse_quantity("-1"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(truncate_to(parse_quantity("-1.1"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(truncate_to(parse_quantity("-1.4"), parse_quantity("0.5")) ==
        parse_quantity("-1"));
      REQUIRE(truncate_to(parse_quantity("-1.5"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
      REQUIRE(truncate_to(parse_quantity("-1.9"), parse_quantity("0.5")) ==
        parse_quantity("-1.5"));
    }

    SUBCASE("tick_size_examples") {
      REQUIRE(truncate_to(parse_quantity("100.12"), parse_quantity("0.25")) ==
        parse_quantity("100"));
      REQUIRE(truncate_to(parse_quantity("100.24"), parse_quantity("0.25")) ==
        parse_quantity("100"));
      REQUIRE(truncate_to(parse_quantity("100.25"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(truncate_to(parse_quantity("100.49"), parse_quantity("0.25")) ==
        parse_quantity("100.25"));
      REQUIRE(truncate_to(parse_quantity("100.5"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(truncate_to(parse_quantity("100.74"), parse_quantity("0.25")) ==
        parse_quantity("100.5"));
      REQUIRE(truncate_to(parse_quantity("100.75"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
      REQUIRE(truncate_to(parse_quantity("100.99"), parse_quantity("0.25")) ==
        parse_quantity("100.75"));
    }

    SUBCASE("decimal_multiples") {
      REQUIRE(truncate_to(parse_quantity("1.234"), parse_quantity("0.1")) ==
        parse_quantity("1.2"));
      REQUIRE(truncate_to(parse_quantity("1.299"), parse_quantity("0.1")) ==
        parse_quantity("1.2"));
      REQUIRE(truncate_to(parse_quantity("1.234"), parse_quantity("0.01")) ==
        parse_quantity("1.23"));
      REQUIRE(truncate_to(parse_quantity("1.239"), parse_quantity("0.01")) ==
        parse_quantity("1.23"));
      REQUIRE(truncate_to(parse_quantity("1.234"), parse_quantity("0.001")) ==
        parse_quantity("1.234"));
    }

    SUBCASE("large_multiples") {
      REQUIRE(truncate_to(parse_quantity("99"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("100"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(truncate_to(parse_quantity("123"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(truncate_to(parse_quantity("199"), parse_quantity("100")) ==
        parse_quantity("100"));
      REQUIRE(truncate_to(parse_quantity("-99"), parse_quantity("100")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-100"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(truncate_to(parse_quantity("-123"), parse_quantity("100")) ==
        parse_quantity("-100"));
      REQUIRE(truncate_to(parse_quantity("-199"), parse_quantity("100")) ==
        parse_quantity("-100"));
    }

    SUBCASE("very_small_multiples") {
      REQUIRE(truncate_to(
        parse_quantity("1.123456"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
      REQUIRE(truncate_to(
        parse_quantity("1.1234569"), parse_quantity("0.000001")) ==
          parse_quantity("1.123456"));
      REQUIRE(truncate_to(
        parse_quantity("-1.123456"), parse_quantity("0.000001")) ==
          parse_quantity("-1.123456"));
      REQUIRE(truncate_to(
        parse_quantity("-1.1234569"), parse_quantity("0.000001")) ==
          parse_quantity("-1.123456"));
    }

    SUBCASE("same_value_and_multiple") {
      REQUIRE(truncate_to(parse_quantity("5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(truncate_to(parse_quantity("0.5"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(truncate_to(parse_quantity("-5"), parse_quantity("5")) ==
        parse_quantity("-5"));
    }

    SUBCASE("value_less_than_multiple") {
      REQUIRE(truncate_to(parse_quantity("0.1"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("9.99"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-0.1"), parse_quantity("10")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-9.99"), parse_quantity("10")) ==
        parse_quantity("0"));
    }

    SUBCASE("precision_edge_cases") {
      auto tiny = parse_quantity("0.000001");
      REQUIRE(truncate_to(parse_quantity("1") + tiny, parse_quantity("1")) ==
        parse_quantity("1"));
      REQUIRE(truncate_to(parse_quantity("1") - tiny, parse_quantity("1")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-1") + tiny, parse_quantity("1")) ==
        parse_quantity("0"));
      REQUIRE(truncate_to(parse_quantity("-1") - tiny, parse_quantity("1")) ==
        parse_quantity("-1"));
    }

    SUBCASE("comparison_with_floor_and_ceil") {
      REQUIRE(truncate_to(parse_quantity("5.7"), parse_quantity("1")) ==
        floor_to(parse_quantity("5.7"), parse_quantity("1")));
      REQUIRE(truncate_to(parse_quantity("-5.7"), parse_quantity("1")) ==
        ceil_to(parse_quantity("-5.7"), parse_quantity("1")));
      REQUIRE(truncate_to(parse_quantity("12.3"), parse_quantity("5")) ==
        floor_to(parse_quantity("12.3"), parse_quantity("5")));
      REQUIRE(truncate_to(parse_quantity("-12.3"), parse_quantity("5")) ==
        ceil_to(parse_quantity("-12.3"), parse_quantity("5")));
    }

    SUBCASE("toward_zero_behavior") {
      REQUIRE(truncate_to(parse_quantity("7.5"), parse_quantity("5")) ==
        parse_quantity("5"));
      REQUIRE(truncate_to(parse_quantity("-7.5"), parse_quantity("5")) ==
        parse_quantity("-5"));
      REQUIRE(truncate_to(parse_quantity("0.75"), parse_quantity("0.5")) ==
        parse_quantity("0.5"));
      REQUIRE(truncate_to(parse_quantity("-0.75"), parse_quantity("0.5")) ==
        parse_quantity("-0.5"));
    }
  }

  TEST_CASE("stream") {
    auto quantity = parse_quantity("12345.678");
    REQUIRE(to_string(quantity) == "12345.678000");
    test_round_trip_shuttle(quantity);
  }
}
