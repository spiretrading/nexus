#include <doctest/doctest.h>
#include "Nexus/Definitions/FixTags.hpp"

using namespace boost;
using namespace Nexus;

TEST_SUITE("FixTags") {
  TEST_CASE("key_constants") {
    REQUIRE(EX_DESTINATION_KEY == 100);
    REQUIRE(EXEC_INST_KEY == 18);
    REQUIRE(MAX_FLOOR_KEY == 111);
    REQUIRE(PEG_DIFFERENCE_KEY == 211);
  }

  TEST_CASE("make_ex_destination") {
    auto tag = make_ex_destination("N");
    REQUIRE(tag.get_key() == EX_DESTINATION_KEY);
    REQUIRE(get<std::string>(tag.get_value()) == "N");
  }

  TEST_CASE("make_exec_inst") {
    auto tag = make_exec_inst(MARKET_PEG);
    REQUIRE(tag.get_key() == EXEC_INST_KEY);
    REQUIRE(get<std::string>(tag.get_value()) == "P");
  }

  TEST_CASE("make_max_floor") {
    auto tag = make_max_floor(Quantity(500));
    REQUIRE(tag.get_key() == MAX_FLOOR_KEY);
    REQUIRE(get<Quantity>(tag.get_value()) == 500);
  }

  TEST_CASE("make_peg_difference") {
    auto tag = make_peg_difference(parse_money("0.05"));
    REQUIRE(tag.get_key() == PEG_DIFFERENCE_KEY);
    REQUIRE(get<Money>(tag.get_value()) == parse_money("0.05"));
  }

  TEST_CASE("peg_constants") {
    REQUIRE(MARKET_PEG == "P");
    REQUIRE(MID_PRICE_PEG == "M");
    REQUIRE(PRIMARY_PEG == "R");
  }
}
