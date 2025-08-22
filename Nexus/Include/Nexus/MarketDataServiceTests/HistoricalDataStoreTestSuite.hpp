#ifndef NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_TEST_SUITE_HPP
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus::MarketDataService::Tests {
  TEST_CASE_TEMPLATE_DEFINE(
      "HistoricalDataStore", T, HistoricalDataStoreTestSuite) {
    using namespace Beam;
    using namespace Beam::Queries;
    using namespace boost;
    using namespace boost::posix_time;
    using namespace Nexus::DefaultCountries;
    using namespace Nexus::DefaultVenues;
    auto data_store = T()();

    SUBCASE("security_info") {
      auto security_a = Security("TST", NYSE);
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
      query_a.SetIndex(security_a);
      query_a.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_a = data_store.load_security_info(query_a);
      REQUIRE(results_a.size() == 1);
      REQUIRE(results_a[0] == info_a);
      auto query_b = SecurityInfoQuery();
      query_b.SetIndex(security_b);
      query_b.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_b = data_store.load_security_info(query_b);
      REQUIRE(results_b.size() == 1);
      REQUIRE(results_b[0] == info_b);
      auto query_c = SecurityInfoQuery();
      query_c.SetIndex(Security("XYZ", NASDAQ));
      query_c.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_c = data_store.load_security_info(query_c);
      REQUIRE(results_c.empty());
      auto query_all = SecurityInfoQuery();
      query_all.SetIndex(US);
      query_all.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_all = data_store.load_security_info(query_all);
      REQUIRE(results_all.size() == 1);
      REQUIRE(results_all[0] == info_a);
    }

    SUBCASE("order_imbalance") {
      auto imbalance_a = SequencedValue(VenueOrderImbalance(
        OrderImbalance(Security("TST", NYSE), Side::BID, 100, 100 * Money::ONE,
          time_from_string("2024-07-09 10:00:00")), NYSE),
        Beam::Queries::Sequence(1));
      auto imbalance_b = SequencedValue(VenueOrderImbalance(
        OrderImbalance(Security("ABC", TSX), Side::ASK, 200, 200 * Money::ONE,
          time_from_string("2024-07-09 10:05:00")), TSX),
        Beam::Queries::Sequence(2));
      auto imbalance_c = SequencedValue(VenueOrderImbalance(
        OrderImbalance(Security("XYZ", NASDAQ), Side::BID, 300,
          300 * Money::ONE, time_from_string("2024-07-09 10:10:00")), NYSE),
        Beam::Queries::Sequence(3));
      data_store.store(imbalance_a);
      auto imbalances = std::vector<SequencedVenueOrderImbalance>();
      imbalances.push_back(imbalance_b);
      imbalances.push_back(imbalance_c);
      data_store.store(imbalances);
      auto query_a = VenueMarketDataQuery();
      query_a.SetIndex(NYSE);
      query_a.SetRange(Beam::Queries::Sequence(1), Beam::Queries::Sequence(1));
      query_a.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_a = data_store.load_order_imbalances(query_a);
      REQUIRE(results_a.size() == 1);
      REQUIRE(*results_a[0] == *imbalance_a);
      auto query_b = VenueMarketDataQuery();
      query_b.SetIndex(TSX);
      query_b.SetRange(Beam::Queries::Sequence(2), Beam::Queries::Sequence(2));
      query_b.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_b = data_store.load_order_imbalances(query_b);
      REQUIRE(results_b.size() == 1);
      REQUIRE(*results_b[0] == *imbalance_b);
      auto query_all = VenueMarketDataQuery();
      query_all.SetIndex(NYSE);
      query_all.SetRange(Range::Total());
      query_all.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_all = data_store.load_order_imbalances(query_all);
      REQUIRE(results_all.size() == 2);
      REQUIRE(*results_all[0] == *imbalance_a);
      REQUIRE(*results_all[1] == *imbalance_c);
    }

    SUBCASE("bbo_quote") {
      auto security = Security("TST", NYSE);
      auto bbo_a = BboQuote(Quote(Money::ONE, 100, Side::BID),
        Quote(Money::ONE + Money::CENT, 100, Side::ASK),
        time_from_string("2024-07-09 12:00:00"));
      auto sequenced_bbo_a = SequencedValue(bbo_a, Beam::Queries::Sequence(1));
      data_store.store(SequencedSecurityBboQuote(
        SecurityBboQuote(bbo_a, security), Beam::Queries::Sequence(1)));
      auto bbo_b = BboQuote(Quote(Money::ONE, 200, Side::BID),
        Quote(Money::ONE + Money::CENT, 200, Side::ASK),
        time_from_string("2024-07-09 12:01:00"));
      auto sequenced_bbo_b = SequencedValue(bbo_b, Beam::Queries::Sequence(2));
      auto bbo_c = BboQuote(Quote(2 * Money::ONE, 300, Side::BID),
        Quote(2 * Money::ONE + Money::CENT, 300, Side::ASK),
        time_from_string("2024-07-09 12:02:00"));
      auto sequenced_bbo_c = SequencedValue(bbo_c, Beam::Queries::Sequence(3));
      auto quotes = std::vector<SequencedSecurityBboQuote>();
      quotes.push_back(SequencedSecurityBboQuote(
        SecurityBboQuote(bbo_b, security), Beam::Queries::Sequence(2)));
      quotes.push_back(SequencedSecurityBboQuote(
        SecurityBboQuote(bbo_c, security), Beam::Queries::Sequence(3)));
      data_store.store(quotes);
      auto other_security = Security("ABC", TSX);
      auto other_bbo = BboQuote(Quote(10 * Money::ONE, 100, Side::BID),
        Quote(10 * Money::ONE + Money::CENT, 100, Side::ASK),
        time_from_string("2024-07-09 12:00:00"));
      data_store.store(SequencedSecurityBboQuote(SecurityBboQuote(
        other_bbo, other_security), Beam::Queries::Sequence(4)));
      auto query_one = SecurityMarketDataQuery();
      query_one.SetIndex(security);
      query_one.SetRange(
        Beam::Queries::Sequence(2), Beam::Queries::Sequence(2));
      query_one.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_one = data_store.load_bbo_quotes(query_one);
      REQUIRE(results_one.size() == 1);
      REQUIRE(results_one[0] == sequenced_bbo_b);
      auto query_all = SecurityMarketDataQuery();
      query_all.SetIndex(security);
      query_all.SetRange(Range::Total());
      query_all.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_all = data_store.load_bbo_quotes(query_all);
      REQUIRE(results_all.size() == 3);
      REQUIRE(results_all[0] == sequenced_bbo_a);
      REQUIRE(results_all[1] == sequenced_bbo_b);
      REQUIRE(results_all[2] == sequenced_bbo_c);
    }

    SUBCASE("book_quote") {
      auto security = Security("TST", NYSE);
      auto book_quote_a = BookQuote("MPID", true, NYSE,
        Quote(Money::ONE, 100, Side::BID),
        time_from_string("2024-07-09 12:00:00"));
      auto sequenced_book_quote_a =
        SequencedValue(book_quote_a, Beam::Queries::Sequence(1));
      data_store.store(SequencedSecurityBookQuote(
        SecurityBookQuote(book_quote_a, security), Beam::Queries::Sequence(1)));
      auto book_quote_b =
        BookQuote("MPID", true, NYSE, Quote(Money::ONE, 200, Side::BID),
          time_from_string("2024-07-09 12:01:00"));
      auto sequenced_book_quote_b =
        SequencedValue(book_quote_b, Beam::Queries::Sequence(2));
      auto book_quote_c = BookQuote("MPID", true, NYSE,
        Quote(Money::ONE + Money::CENT, 300, Side::ASK),
        time_from_string("2024-07-09 12:02:00"));
      auto sequenced_book_quote_c =
        SequencedValue(book_quote_c, Beam::Queries::Sequence(3));
      auto quotes = std::vector<SequencedSecurityBookQuote>();
      quotes.push_back(
        SequencedSecurityBookQuote(SecurityBookQuote(book_quote_b, security),
          Beam::Queries::Sequence(2)));
      quotes.push_back(
        SequencedSecurityBookQuote(SecurityBookQuote(book_quote_c, security),
          Beam::Queries::Sequence(3)));
      data_store.store(quotes);
      auto other_security = Security("ABC", TSX);
      auto other_book_quote =
        BookQuote("MPID", true, TSX, Quote(10 * Money::ONE, 100, Side::BID),
          time_from_string("2024-07-09 12:00:00"));
      data_store.store(SequencedSecurityBookQuote(SecurityBookQuote(
        other_book_quote, other_security), Beam::Queries::Sequence(4)));
      auto query_one = SecurityMarketDataQuery();
      query_one.SetIndex(security);
      query_one.SetRange(
        Beam::Queries::Sequence(2), Beam::Queries::Sequence(2));
      query_one.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_one = data_store.load_book_quotes(query_one);
      REQUIRE(results_one.size() == 1);
      REQUIRE(results_one[0] == sequenced_book_quote_b);
      auto query_all = SecurityMarketDataQuery();
      query_all.SetIndex(security);
      query_all.SetRange(Range::Total());
      query_all.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_all = data_store.load_book_quotes(query_all);
      REQUIRE(results_all.size() == 3);
      REQUIRE(results_all[0] == sequenced_book_quote_a);
      REQUIRE(results_all[1] == sequenced_book_quote_b);
      REQUIRE(results_all[2] == sequenced_book_quote_c);
    }

    SUBCASE("time_and_sale") {
      auto security = Security("TST", NYSE);
      auto time_and_sale_a =
        TimeAndSale(time_from_string("2024-07-09 12:00:00"), Money::ONE, 100,
          TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
          "NYSE", "B1", "S2");
      auto sequenced_time_and_sale_a =
        SequencedValue(time_and_sale_a, Beam::Queries::Sequence(1));
      data_store.store(SequencedSecurityTimeAndSale(
        SecurityTimeAndSale(time_and_sale_a, security),
        Beam::Queries::Sequence(1)));
      auto time_and_sale_b =
        TimeAndSale(time_from_string("2024-07-09 12:01:00"), Money::ONE, 200,
          TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
          "NYSE", "B5", "S12");
      auto sequenced_time_and_sale_b =
        SequencedValue(time_and_sale_b, Beam::Queries::Sequence(2));
      auto time_and_sale_c = TimeAndSale(
        time_from_string("2024-07-09 12:02:00"), Money::ONE + Money::CENT, 300,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
        "NYSE", "B52", "S11");
      auto sequenced_time_and_sale_c =
        SequencedValue(time_and_sale_c, Beam::Queries::Sequence(3));
      auto time_and_sales = std::vector<SequencedSecurityTimeAndSale>();
      time_and_sales.push_back(SequencedSecurityTimeAndSale(
        SecurityTimeAndSale(time_and_sale_b, security),
        Beam::Queries::Sequence(2)));
      time_and_sales.push_back(SequencedSecurityTimeAndSale(
        SecurityTimeAndSale(time_and_sale_c, security),
        Beam::Queries::Sequence(3)));
      data_store.store(time_and_sales);
      auto other_security = Security("ABC", TSX);
      auto other_time_and_sale = TimeAndSale(
        time_from_string("2024-07-09 12:00:00"), 10 * Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, ""),
        "TSX", "B12", "S5");
      data_store.store(SequencedSecurityTimeAndSale(SecurityTimeAndSale(
        other_time_and_sale, other_security), Beam::Queries::Sequence(4)));
      auto query_one = SecurityMarketDataQuery();
      query_one.SetIndex(security);
      query_one.SetRange(
        Beam::Queries::Sequence(2), Beam::Queries::Sequence(2));
      query_one.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_one = data_store.load_time_and_sales(query_one);
      REQUIRE(results_one.size() == 1);
      REQUIRE(results_one[0] == sequenced_time_and_sale_b);
      auto query_all = SecurityMarketDataQuery();
      query_all.SetIndex(security);
      query_all.SetRange(Range::Total());
      query_all.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto results_all = data_store.load_time_and_sales(query_all);
      REQUIRE(results_all.size() == 3);
      REQUIRE(results_all[0] == sequenced_time_and_sale_a);
      REQUIRE(results_all[1] == sequenced_time_and_sale_b);
      REQUIRE(results_all[2] == sequenced_time_and_sale_c);
    }
  }
}

#endif
