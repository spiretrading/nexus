#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/DataStoreMarketDataClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture {
    LocalHistoricalDataStore m_data_store;
    DataStoreMarketDataClient<LocalHistoricalDataStore*> m_client;

    Fixture()
      : m_client(&m_data_store) {}
  };
}

TEST_SUITE("DataStoreMarketDataClient") {
  TEST_CASE("query_ticker_info") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "Test Inc.";
    fixture.m_data_store.store(info);
    auto query = TickerInfoQuery();
    query.set_index(ticker);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto result = fixture.m_client.query(query);
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("query_sequenced_order_imbalances") {
    auto fixture = Fixture();
    auto imbalance = SequencedValue(
      VenueOrderImbalance(OrderImbalance(parse_ticker("TST.TSX"), Side::BID,
        100, Money::ONE, time_from_string("2024-07-10 12:00:00")), TSX),
      Beam::Sequence(1));
    fixture.m_data_store.store(imbalance);
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<SequencedOrderImbalance>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == SequencedValue(**imbalance, imbalance.get_sequence()));
  }

  TEST_CASE("query_order_imbalances") {
    auto fixture = Fixture();
    auto imbalance = SequencedValue(
      VenueOrderImbalance(OrderImbalance(parse_ticker("TST.TSX"), Side::BID,
        100, Money::ONE, time_from_string("2024-07-10 12:00:00")), TSX),
      Beam::Sequence(1));
    fixture.m_data_store.store(imbalance);
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<OrderImbalance>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == *imbalance);
  }

  TEST_CASE("query_sequenced_bbo_quotes") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto quote = SequencedValue(BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    fixture.m_data_store.store(SequencedTickerBboQuote(
      TickerBboQuote(*quote, ticker), quote.get_sequence()));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<SequencedBboQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == quote);
  }

  TEST_CASE("query_bbo_quotes") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto quote = SequencedValue(BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    fixture.m_data_store.store(SequencedTickerBboQuote(
      TickerBboQuote(*quote, ticker), quote.get_sequence()));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<BboQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == *quote);
  }

  TEST_CASE("query_sequenced_book_quotes") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto quote = SequencedValue(
      BookQuote("MPID", true, TSX, make_bid(Money::ONE, 100),
        time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    fixture.m_data_store.store(SequencedTickerBookQuote(
      TickerBookQuote(*quote, ticker), quote.get_sequence()));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<SequencedBookQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == quote);
  }

  TEST_CASE("query_book_quotes") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto quote = SequencedValue(
      BookQuote("MPID", true, TSX, make_bid(Money::ONE, 100),
        time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    fixture.m_data_store.store(SequencedTickerBookQuote(
      TickerBookQuote(*quote, ticker), quote.get_sequence()));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<BookQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == *quote);
  }

  TEST_CASE("query_sequenced_time_and_sales") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto sale = SequencedValue(
      TimeAndSale(time_from_string("2024-07-10 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""),
      Beam::Sequence(1));
    fixture.m_data_store.store(SequencedTickerTimeAndSale(
      TickerTimeAndSale(*sale, ticker), sale.get_sequence()));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == sale);
  }

  TEST_CASE("query_time_and_sales") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("TST.TSX");
    auto sale = SequencedValue(
      TimeAndSale(time_from_string("2024-07-10 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), Beam::Sequence(1));
    fixture.m_data_store.store(SequencedTickerTimeAndSale(
      TickerTimeAndSale(*sale, ticker), sale.get_sequence()));
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto queue = std::make_shared<Queue<TimeAndSale>>();
    fixture.m_client.query(query, queue);
    auto result = queue->pop();
    REQUIRE(result == *sale);
  }

  TEST_CASE("load_snapshot") {
    auto fixture = Fixture();
    auto snapshot = fixture.m_client.load_snapshot(parse_ticker("TST.TSX"));
    REQUIRE(snapshot == TickerSnapshot());
  }

  TEST_CASE("load_session_candlestick") {
    auto fixture = Fixture();
    auto candlestick =
      fixture.m_client.load_session_candlestick(parse_ticker("TST.TSX"));
    REQUIRE(candlestick == PriceCandlestick());
  }

  TEST_CASE("load_ticker_info_from_prefix") {
    auto fixture = Fixture();
    auto result = fixture.m_client.load_ticker_info_from_prefix("T");
    REQUIRE(result.empty());
  }
}
