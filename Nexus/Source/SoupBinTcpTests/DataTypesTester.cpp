#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/DataTypes.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

TEST_SUITE("DataTypes") {
  TEST_CASE("parse_left_padded_alpha_numeric_basic") {
    auto field = "   ABC";
    auto cursor = field;
    auto result = parse_left_padded_alpha_numeric(6, out(cursor));
    REQUIRE(result == "ABC");
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_alpha_numeric_all_spaces") {
    auto field = "      ";
    auto cursor = field;
    auto result = parse_left_padded_alpha_numeric(6, out(cursor));
    REQUIRE(result == "");
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_alpha_numeric_no_padding") {
    auto field = "XYZ123";
    auto cursor = field;
    auto result = parse_left_padded_alpha_numeric(6, out(cursor));
    REQUIRE(result == "XYZ123");
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_alpha_numeric_embedded_spaces") {
    auto field = "  AB C";
    auto cursor = field;
    auto result = parse_left_padded_alpha_numeric(6, out(cursor));
    REQUIRE(result == "AB C");
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_numeric_int") {
    auto field = "   123";
    auto cursor = field;
    auto result = parse_left_padded_numeric<int>(6, out(cursor));
    REQUIRE(result == 123);
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_numeric_negative") {
    auto field = "  -456";
    auto cursor = field;
    auto result = parse_left_padded_numeric<int>(6, out(cursor));
    REQUIRE(result == -456);
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_numeric_all_spaces") {
    auto field = "      ";
    auto cursor = field;
    REQUIRE_THROWS_AS(
      parse_left_padded_numeric<int>(6, out(cursor)), bad_lexical_cast);
    REQUIRE(cursor == field + 6);
  }

  TEST_CASE("parse_left_padded_numeric_leading_zeros") {
    auto field = "000789";
    auto cursor = field;
    auto result = parse_left_padded_numeric<int>(6, out(cursor));
    REQUIRE(result == 789);
    REQUIRE(cursor == field + 6);
  }
}
