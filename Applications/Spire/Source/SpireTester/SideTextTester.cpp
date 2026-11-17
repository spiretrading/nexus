#include <array>
#include <doctest/doctest.h>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("SideText") {
  TEST_CASE("side_text") {
    auto sides = std::array{Side(Side::BID), Side(Side::ASK), Side(Side::NONE)};
    auto texts = std::array{"Bid", "Ask", "None"};
    for(auto i = std::size_t(0); i != sides.size(); ++i) {
      REQUIRE(to_text(sides[i]) == texts[i]);
      REQUIRE(to_text(std::any(sides[i])) == texts[i]);
      REQUIRE((from_text<Side>(texts[i]) == sides[i]));
    }
    REQUIRE((from_text<Side>("Buy") == Side(Side::BID)));
    REQUIRE((from_text<Side>("Sell") == Side(Side::ASK)));
    REQUIRE(!from_text<Side>("Long"));
    REQUIRE(!from_text<Side>("invalid"));
  }

  TEST_CASE("position_side_text") {
    auto sides = std::array{Side(Side::BID), Side(Side::ASK), Side(Side::NONE)};
    auto texts = std::array{"Long", "Short", "Flat"};
    for(auto i = std::size_t(0); i != sides.size(); ++i) {
      auto side = PositionSide(sides[i]);
      REQUIRE(to_text(side) == texts[i]);
      REQUIRE(to_text(std::any(side)) == texts[i]);
      REQUIRE(to_text(AnyRef(side)) == texts[i]);
      REQUIRE((from_text<PositionSide>(texts[i]) == side));
      REQUIRE(to_qvariant(std::any(side)).value<PositionSide>() == side);
      REQUIRE(to_text(PositionSideToken(sides[i])) == texts[i]);
    }
    REQUIRE(!from_text<PositionSide>("Buy"));
    REQUIRE(!from_text<PositionSide>("invalid"));
  }

  TEST_CASE("trade_side_text") {
    auto sides = std::array{Side(Side::BID), Side(Side::ASK), Side(Side::NONE)};
    auto texts = std::array{"Buy", "Sell", "None"};
    for(auto i = std::size_t(0); i != sides.size(); ++i) {
      auto side = TradeSide(sides[i]);
      REQUIRE(to_text(side) == texts[i]);
      REQUIRE(to_text(std::any(side)) == texts[i]);
      REQUIRE(to_text(AnyRef(side)) == texts[i]);
      REQUIRE((from_text<TradeSide>(texts[i]) == side));
      REQUIRE(to_qvariant(std::any(side)).value<TradeSide>() == side);
    }
    REQUIRE(!from_text<TradeSide>("Long"));
    REQUIRE(!from_text<TradeSide>("invalid"));
  }

  TEST_CASE_TEMPLATE("side_text_model", T, Side, PositionSide, TradeSide) {
    auto source = make_local_value_model(T(Side::BID));
    auto model = ToTextModel<T>(source);
    REQUIRE(model.get() == to_text(T(Side::BID)));
    source->set(T(Side::ASK));
    REQUIRE(model.get() == to_text(T(Side::ASK)));
    REQUIRE(model.set(to_text(T(Side::NONE))) == QValidator::Acceptable);
    REQUIRE(source->get() == T(Side::NONE));
    REQUIRE(model.set("invalid") == QValidator::Invalid);
    REQUIRE(source->get() == T(Side::NONE));
  }

  TEST_CASE_TEMPLATE("side_comparison_dispatch", T, PositionSide, TradeSide) {
    auto bid = T(Side::BID);
    auto ask = T(Side::ASK);
    REQUIRE_FALSE(compare(AnyRef(bid), AnyRef(ask)));
    REQUIRE(compare(AnyRef(ask), AnyRef(bid)));
    REQUIRE(is_equal(AnyRef(bid), AnyRef(bid)));
    REQUIRE_FALSE(is_equal(AnyRef(bid), AnyRef(ask)));
    REQUIRE(is_equal(std::any(bid), std::any(bid)));
    REQUIRE_FALSE(is_equal(std::any(bid), std::any(ask)));
  }
}
