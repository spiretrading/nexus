#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto TEST_MPID =
    BookQuote("TSXID", true, DefaultVenues::TSX, Quote(), ptime());
}

TEST_SUITE("IsTopMpidModel") {
  TEST_CASE("constructor_empty") {
    auto top_mpid_prices = std::make_shared<ArrayListModel<TopMpidPrice>>();
    auto mpid = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, mpid, price);
    REQUIRE(!is_top.get());
    SUBCASE("update_top_prices") {
      top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSXV, Money::ONE));
      REQUIRE(!is_top.get());
      top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSX, 2 * Money::ONE));
      REQUIRE(is_top.get());
    }
  }
  TEST_CASE("constructor_missing_mpid") {
    auto top_mpid_prices = std::make_shared<ArrayListModel<TopMpidPrice>>();
    top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSXV, Money::ONE));
    auto mpid = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, mpid, price);
    REQUIRE(!is_top.get());
    SUBCASE("update_mpid") {
      auto updated_mpid = TEST_MPID;
      updated_mpid.m_venue = DefaultVenues::TSXV;
      mpid->set(updated_mpid);
      REQUIRE(is_top.get());
    }
  }
  TEST_CASE("constructor_missing_origin") {
    auto top_mpid_prices = std::make_shared<ArrayListModel<TopMpidPrice>>();
    top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSXV, Money::ONE));
    top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSX, 2 *  Money::ONE));
    SUBCASE("user_order") {
      auto missing_mpid =
        BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
      auto mpid = std::make_shared<LocalValueModel<BookEntry>>(missing_mpid);
      auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
      auto is_top = IsTopMpidModel(top_mpid_prices, mpid, price);
      REQUIRE(!is_top.get());
    }
    SUBCASE("preview") {
      auto missing_mpid = make_limit_order_fields(
        parse_security("TST.TSX"), Side::BID, "TSX", 100, Money::ONE);
      auto mpid = std::make_shared<LocalValueModel<BookEntry>>(missing_mpid);
      auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
      auto is_top = IsTopMpidModel(top_mpid_prices, mpid, price);
      REQUIRE(!is_top.get());
    }
  }
  TEST_CASE("constructor") {
    auto top_mpid_prices = std::make_shared<ArrayListModel<TopMpidPrice>>();
    top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSXV, Money::ONE));
    top_mpid_prices->push(TopMpidPrice(DefaultVenues::TSX, 2 * Money::ONE));
    auto mpid = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, mpid, price);
    REQUIRE(is_top.get());
  }
}
