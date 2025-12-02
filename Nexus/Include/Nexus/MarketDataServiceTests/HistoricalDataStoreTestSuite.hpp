#ifndef NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_TEST_SUITE_HPP
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus::Tests {
  TEST_CASE_TEMPLATE_DEFINE(
      "HistoricalDataStore", T, HistoricalDataStoreTestSuite) {
    using namespace Beam;
    using namespace boost;
    using namespace boost::posix_time;
    using namespace Nexus::DefaultCountries;
    using namespace Nexus::DefaultVenues;
    auto data_store = T()();

    SUBCASE("security_info") {
      auto security_a = Security("TST", ASX);
      auto info_a = SecurityInfo();
      info_a.m_security = security_a;
      info_a.m_name = "Test Inc.";
      info_a.m_sector = "Technology";
      data_store.store(info_a);
      auto security_b = Security("ABC", TSX);
      auto info_b = SecurityInfo();
      info_b.m_security = security_b;
      info_b.m_name = "ABC Corp.";
      info_b.m_sector = "Financials";
      data_store.store(info_b);
      auto query_a = SecurityInfoQuery();
      query_a.set_index(security_a);
      query_a.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_a = data_store.load_security_info(query_a);
      REQUIRE(results_a.size() == 1);
      REQUIRE(results_a[0] == info_a);
      auto query_b = SecurityInfoQuery();
      query_b.set_index(security_b);
      query_b.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_b = data_store.load_security_info(query_b);
      REQUIRE(results_b.size() == 1);
      REQUIRE(results_b[0] == info_b);
      auto query_c = SecurityInfoQuery();
      query_c.set_index(Security("XYZ", TSXV));
      query_c.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_c = data_store.load_security_info(query_c);
      REQUIRE(results_c.empty());
      auto query_all = SecurityInfoQuery();
      query_all.set_index(AU);
      query_all.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_all = data_store.load_security_info(query_all);
      REQUIRE(results_all.size() == 1);
      REQUIRE(results_all[0] == info_a);
    }

    SUBCASE("order_imbalance") {
      auto imbalance_a = SequencedValue(VenueOrderImbalance(
        OrderImbalance(Security("TST", TSX), Side::BID, 100, 100 * Money::ONE,
          time_from_string("2024-07-09 10:00:00")), TSX), Beam::Sequence(1));
      auto imbalance_b = SequencedValue(VenueOrderImbalance(
        OrderImbalance(Security("ABC", TSXV), Side::ASK, 200, 200 * Money::ONE,
          time_from_string("2024-07-09 10:05:00")), TSXV), Beam::Sequence(2));
      auto imbalance_c = SequencedValue(VenueOrderImbalance(
        OrderImbalance(Security("XYZ", TSX), Side::BID, 300,
          300 * Money::ONE, time_from_string("2024-07-09 10:10:00")), TSX),
        Beam::Sequence(3));
      data_store.store(imbalance_a);
      auto imbalances = std::vector<SequencedVenueOrderImbalance>();
      imbalances.push_back(imbalance_b);
      imbalances.push_back(imbalance_c);
      data_store.store(imbalances);
      auto query_a = VenueMarketDataQuery();
      query_a.set_index(TSX);
      query_a.set_range(Beam::Sequence(1), Beam::Sequence(1));
      query_a.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_a = data_store.load_order_imbalances(query_a);
      REQUIRE(results_a.size() == 1);
      REQUIRE(*results_a[0] == *imbalance_a);
      auto query_b = VenueMarketDataQuery();
      query_b.set_index(TSXV);
      query_b.set_range(Beam::Sequence(2), Beam::Sequence(2));
      query_b.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_b = data_store.load_order_imbalances(query_b);
      REQUIRE(results_b.size() == 1);
      REQUIRE(*results_b[0] == *imbalance_b);
      auto query_all = VenueMarketDataQuery();
      query_all.set_index(TSX);
      query_all.set_range(Range::TOTAL);
      query_all.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_all = data_store.load_order_imbalances(query_all);
      REQUIRE(results_all.size() == 2);
      REQUIRE(*results_all[0] == *imbalance_a);
      REQUIRE(*results_all[1] == *imbalance_c);
    }

    SUBCASE("bbo_quote") {
      auto security = Security("TST", ASX);
      auto bbo_a = BboQuote(
        make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
        time_from_string("2024-07-09 12:00:00"));
      auto sequenced_bbo_a = SequencedValue(bbo_a, Beam::Sequence(1));
      data_store.store(SequencedSecurityBboQuote(
        SecurityBboQuote(bbo_a, security), Beam::Sequence(1)));
      auto bbo_b = BboQuote(
        make_bid(Money::ONE, 200), make_ask(Money::ONE + Money::CENT, 200),
        time_from_string("2024-07-09 12:01:00"));
      auto sequenced_bbo_b = SequencedValue(bbo_b, Beam::Sequence(2));
      auto bbo_c = BboQuote(make_bid(2 * Money::ONE, 300),
        make_ask(2 * Money::ONE + Money::CENT, 300),
        time_from_string("2024-07-09 12:02:00"));
      auto sequenced_bbo_c = SequencedValue(bbo_c, Beam::Sequence(3));
      auto quotes = std::vector<SequencedSecurityBboQuote>();
      quotes.push_back(SequencedSecurityBboQuote(
        SecurityBboQuote(bbo_b, security), Beam::Sequence(2)));
      quotes.push_back(SequencedSecurityBboQuote(
        SecurityBboQuote(bbo_c, security), Beam::Sequence(3)));
      data_store.store(quotes);
      auto other_security = Security("ABC", TSX);
      auto other_bbo = BboQuote(make_bid(10 * Money::ONE, 100),
        make_ask(10 * Money::ONE + Money::CENT, 100),
        time_from_string("2024-07-09 12:00:00"));
      data_store.store(SequencedSecurityBboQuote(
        SecurityBboQuote(other_bbo, other_security), Beam::Sequence(4)));
      auto query_one = SecurityMarketDataQuery();
      query_one.set_index(security);
      query_one.set_range(Beam::Sequence(2), Beam::Sequence(2));
      query_one.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_one = data_store.load_bbo_quotes(query_one);
      REQUIRE(results_one.size() == 1);
      REQUIRE(results_one[0] == sequenced_bbo_b);
      auto query_all = SecurityMarketDataQuery();
      query_all.set_index(security);
      query_all.set_range(Range::TOTAL);
      query_all.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_all = data_store.load_bbo_quotes(query_all);
      REQUIRE(results_all.size() == 3);
      REQUIRE(results_all[0] == sequenced_bbo_a);
      REQUIRE(results_all[1] == sequenced_bbo_b);
      REQUIRE(results_all[2] == sequenced_bbo_c);
    }

    SUBCASE("book_quote") {
      auto security = Security("TST", ASX);
      auto book_quote_a = BookQuote("MPID", true, ASX,
        make_bid(Money::ONE, 100), time_from_string("2024-07-09 12:00:00"));
      auto sequenced_book_quote_a =
        SequencedValue(book_quote_a, Beam::Sequence(1));
      data_store.store(SequencedSecurityBookQuote(
        SecurityBookQuote(book_quote_a, security), Beam::Sequence(1)));
      auto book_quote_b = BookQuote("MPID", true, ASX,
        make_bid(Money::ONE, 200), time_from_string("2024-07-09 12:01:00"));
      auto sequenced_book_quote_b =
        SequencedValue(book_quote_b, Beam::Sequence(2));
      auto book_quote_c =
        BookQuote("MPID", true, ASX, make_ask(Money::ONE + Money::CENT, 300),
          time_from_string("2024-07-09 12:02:00"));
      auto sequenced_book_quote_c =
        SequencedValue(book_quote_c, Beam::Sequence(3));
      auto quotes = std::vector<SequencedSecurityBookQuote>();
      quotes.push_back(
        SequencedSecurityBookQuote(SecurityBookQuote(book_quote_b, security),
          Beam::Sequence(2)));
      quotes.push_back(
        SequencedSecurityBookQuote(SecurityBookQuote(book_quote_c, security),
          Beam::Sequence(3)));
      data_store.store(quotes);
      auto other_security = Security("ABC", TSX);
      auto other_book_quote =
        BookQuote("MPID", true, TSX, make_bid(10 * Money::ONE, 100),
          time_from_string("2024-07-09 12:00:00"));
      data_store.store(SequencedSecurityBookQuote(SecurityBookQuote(
        other_book_quote, other_security), Beam::Sequence(4)));
      auto query_one = SecurityMarketDataQuery();
      query_one.set_index(security);
      query_one.set_range(Beam::Sequence(2), Beam::Sequence(2));
      query_one.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_one = data_store.load_book_quotes(query_one);
      REQUIRE(results_one.size() == 1);
      REQUIRE(results_one[0] == sequenced_book_quote_b);
      auto query_all = SecurityMarketDataQuery();
      query_all.set_index(security);
      query_all.set_range(Range::TOTAL);
      query_all.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_all = data_store.load_book_quotes(query_all);
      REQUIRE(results_all.size() == 3);
      REQUIRE(results_all[0] == sequenced_book_quote_a);
      REQUIRE(results_all[1] == sequenced_book_quote_b);
      REQUIRE(results_all[2] == sequenced_book_quote_c);
    }

    SUBCASE("time_and_sale") {
      auto security = Security("TST", ASX);
      auto time_and_sale_a =
        TimeAndSale(time_from_string("2024-07-09 12:00:00"), Money::ONE, 100,
          TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
          "ASX", "B1", "S2");
      auto sequenced_time_and_sale_a =
        SequencedValue(time_and_sale_a, Beam::Sequence(1));
      data_store.store(SequencedSecurityTimeAndSale(
        SecurityTimeAndSale(time_and_sale_a, security), Beam::Sequence(1)));
      auto time_and_sale_b =
        TimeAndSale(time_from_string("2024-07-09 12:01:00"), Money::ONE, 200,
          TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
          "ASX", "B5", "S12");
      auto sequenced_time_and_sale_b =
        SequencedValue(time_and_sale_b, Beam::Sequence(2));
      auto time_and_sale_c = TimeAndSale(
        time_from_string("2024-07-09 12:02:00"), Money::ONE + Money::CENT, 300,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
        "ASX", "B52", "S11");
      auto sequenced_time_and_sale_c =
        SequencedValue(time_and_sale_c, Beam::Sequence(3));
      auto time_and_sales = std::vector<SequencedSecurityTimeAndSale>();
      time_and_sales.push_back(SequencedSecurityTimeAndSale(
        SecurityTimeAndSale(time_and_sale_b, security), Beam::Sequence(2)));
      time_and_sales.push_back(SequencedSecurityTimeAndSale(
        SecurityTimeAndSale(time_and_sale_c, security), Beam::Sequence(3)));
      data_store.store(time_and_sales);
      auto other_security = Security("ABC", TSX);
      auto other_time_and_sale = TimeAndSale(
        time_from_string("2024-07-09 12:00:00"), 10 * Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
        "TSX", "B12", "S5");
      data_store.store(SequencedSecurityTimeAndSale(SecurityTimeAndSale(
        other_time_and_sale, other_security), Beam::Sequence(4)));
      auto query_one = SecurityMarketDataQuery();
      query_one.set_index(security);
      query_one.set_range(Beam::Sequence(2), Beam::Sequence(2));
      query_one.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_one = data_store.load_time_and_sales(query_one);
      REQUIRE(results_one.size() == 1);
      REQUIRE(results_one[0] == sequenced_time_and_sale_b);
      auto query_all = SecurityMarketDataQuery();
      query_all.set_index(security);
      query_all.set_range(Range::TOTAL);
      query_all.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto results_all = data_store.load_time_and_sales(query_all);
      REQUIRE(results_all.size() == 3);
      REQUIRE(results_all[0] == sequenced_time_and_sale_a);
      REQUIRE(results_all[1] == sequenced_time_and_sale_b);
      REQUIRE(results_all[2] == sequenced_time_and_sale_c);
    }
  }
}

#endif
