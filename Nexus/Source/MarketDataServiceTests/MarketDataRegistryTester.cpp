#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;

TEST_SUITE("MarketDataRegistry") {
  TEST_CASE("search_security_info") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto security_a = Security("A", TSX);
    auto info_a = SecurityInfo(security_a, "SECURITY A", "", 100);
    registry.add(info_a);
    auto security_b = Security("B", TSX);
    auto info_b = SecurityInfo(security_b, "SECURITY B", "", 100);
    registry.add(info_b);
    auto security_c = Security("C", TSX);
    auto info_c = SecurityInfo(security_c, "SECURITY C", "", 100);
    registry.add(info_c);
    auto search_result1 = registry.search_security_info("A");
    REQUIRE(search_result1.size() == 1);
    REQUIRE(search_result1.front() == info_a);
    auto search_result2 = registry.search_security_info("sec");
    REQUIRE(search_result2.size() == 3);
    auto find_a1 =
      std::find(search_result2.begin(), search_result2.end(), info_a);
    REQUIRE(find_a1 != search_result2.end());
    auto find_b1 =
      std::find(search_result2.begin(), search_result2.end(), info_b);
    REQUIRE(find_b1 != search_result2.end());
    auto find_c1 =
      std::find(search_result2.begin(), search_result2.end(), info_c);
    REQUIRE(find_c1 != search_result2.end());
    auto search_result3 = registry.search_security_info("D");
    REQUIRE(search_result3.empty());
    auto search_result4 = registry.search_security_info("b.TSX");
    REQUIRE(search_result4.size() == 1);
    REQUIRE(search_result4.front() == info_b);
  }

  TEST_CASE("get_primary_listing") {
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto security_ry_tsx = Security("RY", TSX);
    auto info_ry_tsx =
      SecurityInfo(security_ry_tsx, "Royal Bank", "Financial", 100);
    registry.add(info_ry_tsx);
    auto security_ry_nyse = Security("RY", NYSE);
    auto info_ry_nyse =
      SecurityInfo(security_ry_nyse, "Royal Bank", "Financial", 100);
    registry.add(info_ry_nyse);
    auto security_bmo_tsx = Security("BMO", TSX);
    auto info_bmo_tsx =
      SecurityInfo(security_bmo_tsx, "Bank of Montreal", "Financial", 100);
    registry.add(info_bmo_tsx);
    auto security_bmo_nyse = Security("BMO", NYSE);
    auto info_bmo_nyse =
      SecurityInfo(security_bmo_nyse, "Bank of Montreal", "Financial", 100);
    registry.add(info_bmo_nyse);
    auto primary_listing1 = registry.get_primary_listing(Security("RY", CHIC));
    REQUIRE(primary_listing1 == security_ry_tsx);
    auto primary_listing2 = registry.get_primary_listing(Security("BMO", ARCX));
    REQUIRE(primary_listing2 == security_bmo_nyse);
    auto primary_listing3 = registry.get_primary_listing(Security("NA", TSX));
    REQUIRE(primary_listing3 == Security("NA", TSX));
  }

  TEST_CASE("publish_bbo_quote") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto security = Security("TST", TSX);
    auto bbo_quote =
      SecurityBboQuote(BboQuote(Quote(Money::CENT, 100, Side::BID),
        Quote(2 * Money::CENT, 200, Side::ASK),
        time_from_string("2024-07-12 13:00:00")), security);
    auto published = false;
    registry.publish(bbo_quote, 1, data_store,
      [&] (const auto& sequenced_quote) {
        REQUIRE(sequenced_quote.GetValue() == bbo_quote);
        published = true;
      });
    REQUIRE(published);
  }

  TEST_CASE("publish_book_quote") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto security = Security("TST", TSX);
    auto book_quote = SecurityBookQuote(
      BookQuote("MP1", false, TSX, Quote(Money::CENT, 100, Side::BID),
        time_from_string("2024-07-12 13:00:00")), security);
    auto published = false;
    registry.publish(book_quote, 1, data_store,
      [&] (const auto& sequenced_quote) {
        REQUIRE(sequenced_quote.GetValue() == book_quote);
        published = true;
      });
    REQUIRE(published);
  }

  TEST_CASE("publish_time_and_sale") {
    auto data_store = LocalHistoricalDataStore();
    auto registry =
      MarketDataRegistry(DEFAULT_VENUES, get_default_time_zone_database());
    auto security = Security("TST", TSX);
    auto time_and_sale = SecurityTimeAndSale(
      TimeAndSale(time_from_string("2024-07-12 14:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), security);
    auto published = false;
    registry.publish(time_and_sale, 1, data_store,
      [&] (const auto& sequenced_time_and_sale) {
        REQUIRE(sequenced_time_and_sale.GetValue() == time_and_sale);
        published = true;
      });
    REQUIRE(published);
  }
}
