#include <future>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("ClientHistoricalDataStore") {
  TEST_CASE("load_security_info") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto security = Security("TST", TSX);
    auto info = SecurityInfo();
    info.m_security = security;
    info.m_name = "Test Inc.";
    info.m_sector = "Technology";
    auto query = SecurityInfoQuery();
    query.SetIndex(security);
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_security_info(query);
    });
    auto operation = operations->Pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::SecurityInfoQueryOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.GetIndex() == security);
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
    query.SetIndex(TSX);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_order_imbalances(query);
    });
    auto operation = operations->Pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.GetIndex() == TSX);
    auto imbalance = SequencedValue(OrderImbalance(
      OrderImbalance(Security("TST", TSX), Side::BID, 100, Money::ONE,
        time_from_string("2024-07-10 12:00:00"))), Beam::Queries::Sequence(1));
    query_operation->m_queue.Push(imbalance);
    query_operation->m_queue.Break();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == imbalance);
  }

  TEST_CASE("load_bbo_quotes") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto security = Security("TST", TSX);
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_bbo_quotes(query);
    });
    auto operation = operations->Pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedBboQuoteOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.GetIndex() == security);
    auto quote = SequencedValue(BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE + Money::CENT, 100, Side::ASK),
      time_from_string("2024-07-10 12:00:00")), Beam::Queries::Sequence(1));
    query_operation->m_queue.Push(quote);
    query_operation->m_queue.Break();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == quote);
  }

  TEST_CASE("load_book_quotes") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto security = Security("TST", TSX);
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_book_quotes(query);
    });
    auto operation = operations->Pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedBookQuoteOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.GetIndex() == security);
    auto quote = SequencedValue(
      BookQuote("MPID", true, TSX, Quote(Money::ONE, 100, Side::BID),
        time_from_string("2024-07-10 12:00:00")), Beam::Queries::Sequence(1));
    query_operation->m_queue.Push(quote);
    query_operation->m_queue.Break();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == quote);
  }

  TEST_CASE("load_time_and_sales") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto data_store = ClientHistoricalDataStore<TestMarketDataClient*>(&client);
    auto security = Security("TST", TSX);
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto load_routine = std::async(std::launch::async, [&] {
      return data_store.load_time_and_sales(query);
    });
    auto operation = operations->Pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QuerySequencedTimeAndSaleOperation>(
        &*operation);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.GetIndex() == security);
    auto sale = SequencedValue(
      TimeAndSale(time_from_string("2024-07-10 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), Beam::Queries::Sequence(1));
    query_operation->m_queue.Push(sale);
    query_operation->m_queue.Break();
    auto result = load_routine.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == sale);
  }
}
