#include <future>
#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("TestMarketDataClient") {
  TEST_CASE("query") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    client.query(query, imbalances);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryOrderImbalanceOperation>(
        &*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == TSX);
  }

  TEST_CASE("multiple_streaming_queries") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto sequenced_imbalances =
      std::make_shared<Queue<SequencedOrderImbalance>>();
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    client.query(query, imbalances);
    auto sequenced_query = VenueMarketDataQuery();
    sequenced_query.set_index(TSXV);
    client.query(sequenced_query, sequenced_imbalances);
    auto op1 = operations->pop();
    auto oi_op =
      std::get_if<TestMarketDataClient::QueryOrderImbalanceOperation>(&*op1);
    REQUIRE(oi_op);
    REQUIRE(oi_op->m_query.get_index() == TSX);
    auto op2 = operations->pop();
    auto seq_op =
      std::get_if<TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
        &*op2);
    REQUIRE(seq_op);
    REQUIRE(seq_op->m_query.get_index() == TSXV);
  }

  TEST_CASE("streaming_query_after_close") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    client.close();
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto query = VenueMarketDataQuery();
    query.set_index(TSX);
    client.query(query, imbalances);
    REQUIRE_FALSE(operations->try_pop());
    REQUIRE_THROWS_AS(imbalances->pop(), PipeBrokenException);
  }

  TEST_CASE("load_snapshot_after_close_throws") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    client.close();
    REQUIRE_THROWS_AS(client.load_snapshot(Ticker()), EndOfFileException);
  }
}
