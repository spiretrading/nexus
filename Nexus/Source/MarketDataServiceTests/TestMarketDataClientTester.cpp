#include <future>
#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("TestMarketDataClient") {
  TEST_CASE("query") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto query = VenueMarketDataQuery();
    query.SetIndex(NYSE);
    client.query(query, imbalances);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryOrderImbalanceOperation>(
        &*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == NYSE);
  }

  TEST_CASE("multiple_streaming_queries") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto sequenced_imbalances =
      std::make_shared<Queue<SequencedOrderImbalance>>();
    auto query = VenueMarketDataQuery();
    query.SetIndex(NYSE);
    client.query(query, imbalances);
    auto sequenced_query = VenueMarketDataQuery();
    sequenced_query.SetIndex(NASDAQ);
    client.query(sequenced_query, sequenced_imbalances);
    auto op1 = operations->Pop();
    auto oi_op =
      std::get_if<TestMarketDataClient::QueryOrderImbalanceOperation>(&*op1);
    REQUIRE(oi_op);
    REQUIRE(oi_op->m_query.GetIndex() == NYSE);
    auto op2 = operations->Pop();
    auto seq_op =
      std::get_if<TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
        &*op2);
    REQUIRE(seq_op);
    REQUIRE(seq_op->m_query.GetIndex() == NASDAQ);
  }

  TEST_CASE("streaming_query_after_close") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    client.close();
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto query = VenueMarketDataQuery();
    query.SetIndex(NYSE);
    client.query(query, imbalances);
    REQUIRE_FALSE(operations->TryPop());
    REQUIRE_THROWS_AS(imbalances->Pop(), PipeBrokenException);
  }

  TEST_CASE("streaming_query_during_close_race") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto started = std::atomic<bool>(false);
    auto query_async = std::async(std::launch::async, [&] {
      auto query = VenueMarketDataQuery();
      query.SetIndex(NYSE);
      started = true;
      client.query(query, imbalances);
    });
    while(!started) {}
    client.close();
    query_async.get();
    if(auto op = operations->TryPop()) {
      auto oi_op = std::get_if<
        TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(&**op);
      REQUIRE(oi_op->m_query.GetIndex() == NYSE);
    } else {
      REQUIRE_THROWS_AS(imbalances->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("load_snapshot_after_close_throws") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = TestMarketDataClient(operations);
    client.close();
    REQUIRE_THROWS_AS(client.load_snapshot(Security()), IO::EndOfFileException);
  }
}
