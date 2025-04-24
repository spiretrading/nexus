#include <doctest/doctest.h>
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"

using namespace Spire;

TEST_SUITE("OrderTaskArgumentsMatch") {
  TEST_CASE("match_a_single_word") {
    auto words = std::vector<QString>{"ASX", "TradeMatch", "Primary", "Peg",
      "Bid", "ASX", "ASXT", "Pegged", "Buy", "Adjustable", "Day",
      "ExecInst", "PegDifference"};
    auto query = "asx";
    matches(query, words);
    REQUIRE(matches(query, words));
    query = "as";
    REQUIRE(matches(query, words));
    query = "asxx";
    REQUIRE(!matches(query, words));
  }

  TEST_CASE("match_multiple_words") {
    auto words = std::vector<QString>{"ASX", "TradeMatch", "Limit", "Bid",
      "ASX", "ASXT", "Limit", "Buy", "Adjustable", "Day", "MaxFloor"};
    auto query = "asx";
    REQUIRE(matches(query, words));
    query = "asx limit";
    REQUIRE(matches(query, words));
    query = "asx limit sell";
    REQUIRE(!matches(query, words));
  }

  TEST_CASE("match_repeat_words") {
    auto words = std::vector<QString>{"ASX", "TradeMatch", "Primary", "Peg",
      "Bid", "ASX", "ASXT", "Pegged", "Buy", "Adjustable", "Day", "ExecInst",
      "PegDifference"};
    auto query = "peg";
    REQUIRE(matches(query, words));
    query = "peg peg";
    REQUIRE(matches(query, words));
    query = "peg peg peg";
    REQUIRE(matches(query, words));
    query = "peg peg peg peg";
    REQUIRE(!matches(query, words));
  }

  TEST_CASE("match_multiple_words_out_of_order") {
    auto words = std::vector<QString>{"ASX", "TradeMatch", "Primary", "Peg",
      "Bid", "ASX", "ASXT", "Pegged", "Buy", "Adjustable", "Day", "ExecInst",
      "PegDifference"};
    auto query = "buy";
    REQUIRE(matches(query, words));
    query = "buy asxt";
    REQUIRE(matches(query, words));
    query = "buy asxt pegged";
    REQUIRE(matches(query, words));
    query = "buy asxt pegged mid";
    REQUIRE(!matches(query, words));
  }

  TEST_CASE("match_prefixes") {
    auto words = std::vector<QString>{"NEO", "Lit", "Limit", "Bid", "CSE",
      "NEOE", "TSX", "TSXV", "NEOE", "Limit", "Buy", "Adjustable", "Day",
      "MaxFloor"};
    auto query = "li";
    REQUIRE(matches(query, words));
    query = "li li";
    REQUIRE(matches(query, words));
    query = "li li lit";
    REQUIRE(matches(query, words));
    query = "li li lit b";
    REQUIRE(matches(query, words));
    query = "li li lit b e";
    REQUIRE(!matches(query, words));
  }
}
