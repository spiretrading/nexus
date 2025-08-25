#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/DataStoreMarketDataClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Serialization::Tests;
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
  TEST_CASE("query_security_info") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto info = SecurityInfo();
    info.m_security = security;
    info.m_name = "Test Inc.";
    info.m_sector = "Technology";
    fixture.m_data_store.store(info);
    auto query = SecurityInfoQuery();
    query.SetIndex(security);
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto result = fixture.m_client.query(query);
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("query_sequenced_order_imbalances") {
    auto fixture = Fixture();
    auto imbalance = SequencedValue(
      VenueOrderImbalance(OrderImbalance(Security("TST", TSX), Side::BID, 100,
        Money::ONE, time_from_string("2024-07-10 12:00:00")), TSX),
      Beam::Queries::Sequence(1));
    fixture.m_data_store.store(imbalance);
    auto query = VenueMarketDataQuery();
    query.SetIndex(TSX);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<SequencedOrderImbalance>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == SequencedValue(**imbalance, imbalance.GetSequence()));
  }

  TEST_CASE("query_order_imbalances") {
    auto fixture = Fixture();
    auto imbalance = SequencedValue(
      VenueOrderImbalance(OrderImbalance(Security("TST", TSX), Side::BID, 100,
        Money::ONE, time_from_string("2024-07-10 12:00:00")), TSX),
      Beam::Queries::Sequence(1));
    fixture.m_data_store.store(imbalance);
    auto query = VenueMarketDataQuery();
    query.SetIndex(TSX);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<OrderImbalance>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == *imbalance);
  }

  TEST_CASE("query_sequenced_bbo_quotes") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto quote = SequencedValue(BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE + Money::CENT, 100, Side::ASK),
      time_from_string("2024-07-10 12:00:00")), Beam::Queries::Sequence(1));
    fixture.m_data_store.store(SequencedSecurityBboQuote(
      SecurityBboQuote(quote.GetValue(), security), quote.GetSequence()));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<SequencedBboQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == quote);
  }

  TEST_CASE("query_bbo_quotes") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto quote = SequencedValue(BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE + Money::CENT, 100, Side::ASK),
      time_from_string("2024-07-10 12:00:00")), Beam::Queries::Sequence(1));
    fixture.m_data_store.store(SequencedSecurityBboQuote(
      SecurityBboQuote(quote.GetValue(), security), quote.GetSequence()));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<BboQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == *quote);
  }

  TEST_CASE("query_sequenced_book_quotes") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto quote = SequencedValue(
      BookQuote("MPID", true, TSX, Quote(Money::ONE, 100, Side::BID),
        time_from_string("2024-07-10 12:00:00")), Beam::Queries::Sequence(1));
    fixture.m_data_store.store(SequencedSecurityBookQuote(
      SecurityBookQuote(quote.GetValue(), security), quote.GetSequence()));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<SequencedBookQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == quote);
  }

  TEST_CASE("query_book_quotes") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto quote = SequencedValue(
      BookQuote("MPID", true, TSX, Quote(Money::ONE, 100, Side::BID),
        time_from_string("2024-07-10 12:00:00")), Beam::Queries::Sequence(1));
    fixture.m_data_store.store(SequencedSecurityBookQuote(
      SecurityBookQuote(quote.GetValue(), security), quote.GetSequence()));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<BookQuote>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == *quote);
  }

  TEST_CASE("query_sequenced_time_and_sales") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto sale = SequencedValue(
      TimeAndSale(time_from_string("2024-07-10 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""),
      Beam::Queries::Sequence(1));
    fixture.m_data_store.store(SequencedSecurityTimeAndSale(
      SecurityTimeAndSale(sale.GetValue(), security), sale.GetSequence()));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<SequencedTimeAndSale>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == sale);
  }

  TEST_CASE("query_time_and_sales") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto sale = SequencedValue(
      TimeAndSale(time_from_string("2024-07-10 12:00:00"), Money::ONE, 100,
        TimeAndSale::Condition(), "TSX", "", ""), Beam::Queries::Sequence(1));
    fixture.m_data_store.store(SequencedSecurityTimeAndSale(
      SecurityTimeAndSale(sale.GetValue(), security), sale.GetSequence()));
    auto query = SecurityMarketDataQuery();
    query.SetIndex(security);
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto queue = std::make_shared<Queue<TimeAndSale>>();
    fixture.m_client.query(query, queue);
    auto result = queue->Pop();
    REQUIRE(result == *sale);
  }

  TEST_CASE("load_snapshot") {
    auto fixture = Fixture();
    auto snapshot = fixture.m_client.load_snapshot(Security("TST", TSX));
    REQUIRE(snapshot == SecuritySnapshot());
  }

  TEST_CASE("load_technicals") {
    auto fixture = Fixture();
    auto technicals = fixture.m_client.load_technicals(Security("TST", TSX));
    TestJsonEquality(technicals, SecurityTechnicals());
  }

  TEST_CASE("load_security_info_from_prefix") {
    auto fixture = Fixture();
    auto result = fixture.m_client.load_security_info_from_prefix("T");
    REQUIRE(result.empty());
  }
}
