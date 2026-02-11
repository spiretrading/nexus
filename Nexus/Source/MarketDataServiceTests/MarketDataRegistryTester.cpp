#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("MarketDataRegistry") {
  TEST_CASE("search_ticker_info") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto ticker_a = parse_ticker("A.TSX");
    auto info_a = TickerInfo();
    info_a.m_ticker = ticker_a;
    info_a.m_name = "TICKER A";
    info_a.m_board_lot = 100;
    registry.add(info_a);
    auto ticker_b = parse_ticker("B.TSX");
    auto info_b = TickerInfo();
    info_b.m_ticker = ticker_b;
    info_b.m_name = "TICKER B";
    info_b.m_board_lot = 100;
    registry.add(info_b);
    auto ticker_c = parse_ticker("C.TSX");
    auto info_c = TickerInfo();
    info_c.m_ticker = ticker_c;
    info_c.m_name = "TICKER C";
    info_c.m_board_lot = 100;
    registry.add(info_c);
    auto search_result1 = registry.search_ticker_info("A");
    REQUIRE(search_result1.size() == 1);
    REQUIRE(search_result1.front() == info_a);
    auto search_result2 = registry.search_ticker_info("tic");
    REQUIRE(search_result2.size() == 3);
    REQUIRE(std::ranges::contains(search_result2, info_a));
    REQUIRE(std::ranges::contains(search_result2, info_b));
    REQUIRE(std::ranges::contains(search_result2, info_c));
    auto search_result3 = registry.search_ticker_info("D");
    REQUIRE(search_result3.empty());
    auto search_result4 = registry.search_ticker_info("b.TSX");
    REQUIRE(search_result4.size() == 1);
    REQUIRE(search_result4.front() == info_b);
  }

  TEST_CASE("get_primary_listing") {
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto ticker_ry_tsx = parse_ticker("RY.TSX");
    auto info_ry_tsx = TickerInfo();
    info_ry_tsx.m_ticker = ticker_ry_tsx;
    info_ry_tsx.m_name = "Royal Bank";
    info_ry_tsx.m_board_lot = 100;
    registry.add(info_ry_tsx);
    auto ticker_ry_asx = parse_ticker("RY.ASX");
    auto info_ry_asx = TickerInfo();
    info_ry_asx.m_ticker = ticker_ry_asx;
    info_ry_asx.m_name = "Royal Bank";
    info_ry_asx.m_board_lot = 100;
    registry.add(info_ry_asx);
    auto ticker_bmo_tsx = parse_ticker("BMO.TSX");
    auto info_bmo_tsx = TickerInfo();
    info_bmo_tsx.m_ticker = ticker_bmo_tsx;
    info_bmo_tsx.m_name = "Bank of Montreal";
    info_bmo_tsx.m_board_lot = 100;
    registry.add(info_bmo_tsx);
    auto ticker_bmo_asx = parse_ticker("BMO.ASX");
    auto info_bmo_asx = TickerInfo();
    info_bmo_asx.m_ticker = ticker_bmo_asx;
    info_bmo_asx.m_name = "Bank of Montreal";
    info_bmo_asx.m_board_lot = 100;
    registry.add(info_bmo_asx);
    auto primary_listing1 =
      registry.get_primary_listing(parse_ticker("RY.CHIC"));
    REQUIRE(primary_listing1 == ticker_ry_tsx);
    auto primary_listing2 =
      registry.get_primary_listing(parse_ticker("BMO.CXA"));
    REQUIRE(primary_listing2 == ticker_bmo_asx);
    auto primary_listing3 =
      registry.get_primary_listing(parse_ticker("NA.TSX"));
    REQUIRE(primary_listing3 == parse_ticker("NA.TSX"));
  }

  TEST_CASE("publish_bbo_quote") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto ticker = parse_ticker("TST.TSX");
    auto bbo_quote = TickerBboQuote(
      BboQuote(make_bid(Money::CENT, 100), make_ask(2 * Money::CENT, 200),
        time_from_string("2024-07-12 13:00:00")), ticker);
    auto published = false;
    registry.publish(bbo_quote, 1, data_store,
      [&] (const auto& sequenced_quote) {
        REQUIRE(*sequenced_quote == bbo_quote);
        published = true;
      });
    REQUIRE(published);
  }

  TEST_CASE("publish_book_quote") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto ticker = parse_ticker("TST.TSX");
    auto book_quote = TickerBookQuote(
      BookQuote("MP1", false, TSX, make_bid(Money::CENT, 100),
        time_from_string("2024-07-12 13:00:00")), ticker);
    auto published = false;
    registry.publish(book_quote, 1, data_store,
      [&] (const auto& sequenced_quote) {
        REQUIRE(*sequenced_quote == book_quote);
        published = true;
      });
    REQUIRE(published);
  }

  TEST_CASE("publish_time_and_sale") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto ticker = parse_ticker("TST.TSX");
    auto time_and_sale = TickerTimeAndSale(
      TimeAndSale(time_from_string("2024-07-12 14:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), ticker);
    auto published = false;
    registry.publish(time_and_sale, 1, data_store,
      [&] (const auto& sequenced_time_and_sale) {
        REQUIRE(*sequenced_time_and_sale == time_and_sale);
        published = true;
      });
    REQUIRE(published);
  }
}
