#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  auto TEST_MPID =
    Mpid("TSXID", DefaultMarkets::TSX(), Mpid::Origin::BOOK_QUOTE);
}

TEST_SUITE("IsTopMpidModel") {
  TEST_CASE("constructor_empty") {
    auto top_mpid_levels = std::make_shared<ArrayListModel<TopMpidLevel>>();
    auto mpid = std::make_shared<LocalValueModel<Mpid>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_levels, mpid, price);
    REQUIRE(!is_top.get());
    SUBCASE("update_top_levels") {
      top_mpid_levels->push(
        TopMpidLevel(DefaultMarkets::TSXV(), 1, Money::ONE));
      REQUIRE(!is_top.get());
      top_mpid_levels->push(
        TopMpidLevel(DefaultMarkets::TSX(), 2, 2 * Money::ONE));
      REQUIRE(is_top.get());
    }
  }
  TEST_CASE("constructor_missing_mpid") {
    auto top_mpid_levels = std::make_shared<ArrayListModel<TopMpidLevel>>();
    top_mpid_levels->push(TopMpidLevel(DefaultMarkets::TSXV(), 1, Money::ONE));
    auto mpid = std::make_shared<LocalValueModel<Mpid>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_levels, mpid, price);
    REQUIRE(!is_top.get());
  }
  TEST_CASE("constructor_missing_origin") {
    auto top_mpid_levels = std::make_shared<ArrayListModel<TopMpidLevel>>();
    top_mpid_levels->push(TopMpidLevel(DefaultMarkets::TSXV(), 1, Money::ONE));
    top_mpid_levels->push(
      TopMpidLevel(DefaultMarkets::TSX(), 2, 2 *  Money::ONE));
    auto missing_mpid = TEST_MPID;
    SUBCASE("user_order") {
      missing_mpid.m_origin = Mpid::Origin::USER_ORDER;
      auto mpid = std::make_shared<LocalValueModel<Mpid>>(missing_mpid);
      auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
      auto is_top = IsTopMpidModel(top_mpid_levels, mpid, price);
      REQUIRE(!is_top.get());
    }
    SUBCASE("preview") {
      missing_mpid.m_origin = Mpid::Origin::PREVIEW;
      auto mpid = std::make_shared<LocalValueModel<Mpid>>(missing_mpid);
      auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
      auto is_top = IsTopMpidModel(top_mpid_levels, mpid, price);
      REQUIRE(!is_top.get());
    }
  }
  TEST_CASE("constructor") {
    auto top_mpid_levels = std::make_shared<ArrayListModel<TopMpidLevel>>();
    top_mpid_levels->push(TopMpidLevel(DefaultMarkets::TSXV(), 1, Money::ONE));
    top_mpid_levels->push(
      TopMpidLevel(DefaultMarkets::TSX(), 2, 2 * Money::ONE));
    auto mpid = std::make_shared<LocalValueModel<Mpid>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_levels, mpid, price);
    REQUIRE(is_top.get());
  }
}
