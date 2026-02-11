#include <future>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("ClientHistoricalDataStore") {
  TEST_CASE("load_ticker_info") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto ticker = parse_ticker("TST.TSX");
    auto info = TickerInfo();
    info.m_ticker = ticker;
    info.m_name = "Test Inc.";
    auto query = TickerInfoQuery();
    query.set_index(ticker);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_ticker_info(query);
    });
    auto operation = operations->pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::TickerInfoQueryOperation>(&*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.get_index() == ticker);
    query_operation->m_result.set({info});
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("load_order_imbalances") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_order_imbalances(query);
    });
    auto operation = operations->pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.get_index() == TSX);
    auto imbalance = SequencedValue(OrderImbalance(
      parse_ticker("TST.TSX"), Side::BID, 100, Money::ONE,
        time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    query_operation->m_queue.push(imbalance);
    query_operation->m_queue.close();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == imbalance);
  }

  TEST_CASE("load_bbo_quotes") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto ticker = parse_ticker("TST.TSX");
    auto query = TickerMarketDataQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_bbo_quotes(query);
    });
    auto operation = operations->pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedBboQuoteOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.get_index() == ticker);
    auto quote = SequencedValue(BboQuote(
      make_bid(Money::ONE, 100), make_ask(Money::ONE + Money::CENT, 100),
      time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    query_operation->m_queue.push(quote);
    query_operation->m_queue.close();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == quote);
  }

  TEST_CASE("load_book_quotes") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto ticker = parse_ticker("TST.TSX");
    auto query = TickerMarketDataQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_book_quotes(query);
    });
    auto operation = operations->pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedBookQuoteOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.get_index() == ticker);
    auto quote =
      SequencedValue(BookQuote("MPID", true, TSX, make_bid(Money::ONE, 100),
        time_from_string("2024-07-10 12:00:00")), Beam::Sequence(1));
    query_operation->m_queue.push(quote);
    query_operation->m_queue.close();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == quote);
  }

  TEST_CASE("load_time_and_sales") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto ticker = parse_ticker("TST.TSX");
    auto query = TickerMarketDataQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_time_and_sales(query);
    });
    auto operation = operations->pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedTimeAndSaleOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.get_index() == ticker);
    auto sale = SequencedValue(
      TimeAndSale(time_from_string("2024-07-10 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), Beam::Sequence(1));
    query_operation->m_queue.push(sale);
    query_operation->m_queue.close();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == sale);
  }
}
