#include <doctest/doctest.h>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/ParseFeeTable.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("ParseFeeTable") {
  TEST_CASE("unordered_map_basic") {
    auto yaml = YAML::Load(
      "A: 1.23\n"
      "B: 4.56\n"
      "C: 7.89\n");
    auto table = std::unordered_map<std::string, Money>();
    parse_fee_table<Money>(yaml, out(table));
    REQUIRE(table.size() == 3);
    REQUIRE(table.at("A") == parse_money("1.23"));
    REQUIRE(table.at("B") == parse_money("4.56"));
    REQUIRE(table.at("C") == parse_money("7.89"));
  }

  TEST_CASE("unordered_map_named_node") {
    auto yaml = YAML::Load(
      "fees:\n"
      "  X: 10.00\n"
      "  Y: 20.00\n");
    auto table = std::unordered_map<std::string, Money>();
    parse_fee_table<Money>(yaml, "fees", out(table));
    REQUIRE(table.size() == 2);
    REQUIRE(table.at("X") == parse_money("10.00"));
    REQUIRE(table.at("Y") == parse_money("20.00"));
  }

  TEST_CASE("unordered_map_missing_node") {
    auto yaml = YAML::Load("other: {}");
    auto table = std::unordered_map<std::string, Money>();
    REQUIRE_THROWS_AS(
      parse_fee_table<Money>(yaml, "fees", out(table)), std::runtime_error);
  }

  TEST_CASE("array_basic") {
    auto yaml = YAML::Load("[1.1, 2.2, 3.3]");
    auto table = std::array<Money, 3>();
    parse_fee_table<Money, 3>(yaml, out(table));
    REQUIRE(table[0] == parse_money("1.1"));
    REQUIRE(table[1] == parse_money("2.2"));
    REQUIRE(table[2] == parse_money("3.3"));
  }

  TEST_CASE("array_named_node") {
    auto yaml = YAML::Load("fees: [5.5, 6.6, 7.7]");
    auto table = std::array<Money, 3>();
    parse_fee_table<Money, 3>(yaml, "fees", out(table));
    REQUIRE(table[0] == parse_money("5.5"));
    REQUIRE(table[1] == parse_money("6.6"));
    REQUIRE(table[2] == parse_money("7.7"));
  }

  TEST_CASE("array_invalid_size") {
    auto yaml = YAML::Load("[1.1, 2.2]");
    auto table = std::array<Money, 3>();
    REQUIRE_THROWS_AS(
      (parse_fee_table<Money, 3>(yaml, out(table))), std::runtime_error);
  }

  TEST_CASE("2d_array_basic") {
    auto yaml = YAML::Load("[[1, 2], [3, 4]]");
    auto table = std::array<std::array<Quantity, 2>, 2>();
    parse_fee_table<Quantity, 2, 2>(yaml, out(table));
    REQUIRE(table[0][0] == parse_quantity("1"));
    REQUIRE(table[0][1] == parse_quantity("2"));
    REQUIRE(table[1][0] == parse_quantity("3"));
    REQUIRE(table[1][1] == parse_quantity("4"));
  }

  TEST_CASE("2d_array_named_node") {
    auto yaml = YAML::Load("fees: [[10, 20], [30, 40]]");
    auto table = std::array<std::array<Quantity, 2>, 2>();
    parse_fee_table<Quantity, 2, 2>(yaml, "fees", out(table));
    REQUIRE(table[0][0] == parse_quantity("10"));
    REQUIRE(table[0][1] == parse_quantity("20"));
    REQUIRE(table[1][0] == parse_quantity("30"));
    REQUIRE(table[1][1] == parse_quantity("40"));
  }

  TEST_CASE("2d_array_invalid_row_size") {
    auto yaml = YAML::Load("[[1, 2], [3, 4], [5, 6]]");
    auto table = std::array<std::array<Quantity, 2>, 2>();
    REQUIRE_THROWS_AS((parse_fee_table<Quantity, 2, 2>(yaml, out(table))),
      std::runtime_error);
  }

  TEST_CASE("2d_array_invalid_column_size") {
    auto yaml = YAML::Load("[[1, 2, 3], [4, 5, 6]]");
    auto table = std::array<std::array<Quantity, 2>, 2>();
    REQUIRE_THROWS_AS((parse_fee_table<Quantity, 2, 2>(yaml, out(table))),
      std::runtime_error);
  }
}
