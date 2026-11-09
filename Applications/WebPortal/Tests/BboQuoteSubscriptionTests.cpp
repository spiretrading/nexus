#include <Beam/Routines/RoutineHandler.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"
#include "WebPortal/BboQuoteSubscription.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

TEST_SUITE("BboQuoteSubscription") {
  TEST_CASE("snapshot_and_updates") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = MarketDataClient(
      std::in_place_type<TestMarketDataClient>, operations);
    auto ticker = parse_ticker("TST.TSX");
    auto subscription = BboQuoteSubscription(client, ticker);
    auto operation = operations->pop();
    auto& query = std::get<
      TestMarketDataClient::QuerySequencedBboQuoteOperation>(*operation);
    REQUIRE(query.m_query.get_index() == ticker);
    REQUIRE(query.m_query.get_range() == Range::TOTAL);
    REQUIRE(query.m_query.get_snapshot_limit() == SnapshotLimit::from_tail(1));
    auto quote = SequencedBboQuote(BboQuote(make_bid(Money(10), 100),
      make_ask(Money(11), 200), time_from_string("2026-11-08 10:00:00")),
      Sequence(9007199254740993ULL));
    query.m_queue.push(quote);
    REQUIRE(subscription.get_snapshot() == std::vector{quote});
    auto update = SequencedBboQuote(*quote,
      increment(quote.get_sequence()));
    query.m_queue.push(update);
    REQUIRE(subscription.pop() == update);
    REQUIRE(subscription.get_snapshot().empty());
  }

  TEST_CASE("late_initial_quote_and_independent_subscriptions") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = MarketDataClient(
      std::in_place_type<TestMarketDataClient>, operations);
    auto first = BboQuoteSubscription(client, parse_ticker("TST.TSX"));
    auto second = BboQuoteSubscription(client, parse_ticker("OTHER.TSX"));
    auto first_operation = operations->pop();
    auto second_operation = operations->pop();
    auto& first_query = std::get<
      TestMarketDataClient::QuerySequencedBboQuoteOperation>(*first_operation);
    auto& second_query = std::get<
      TestMarketDataClient::QuerySequencedBboQuoteOperation>(*second_operation);
    REQUIRE(first.get_snapshot().empty());
    auto quote = SequencedBboQuote(BboQuote(), Sequence(10));
    first_query.m_queue.push(quote);
    REQUIRE(first.pop() == quote);
    first.close();
    REQUIRE_THROWS_AS(first_query.m_queue.push(quote), PipeBrokenException);
    second_query.m_queue.push(quote);
    REQUIRE(second.pop() == quote);
  }

  TEST_CASE("cancel_pending_reader") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = MarketDataClient(
      std::in_place_type<TestMarketDataClient>, operations);
    auto subscription = BboQuoteSubscription(client, parse_ticker("TST.TSX"));
    auto result = Queue<SequencedBboQuote>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        result.push(subscription.pop());
      } catch(const std::exception&) {
        result.close(std::current_exception());
      }
    }));
    subscription.close();
    REQUIRE_THROWS_AS(result.pop(), PipeBrokenException);
    reader.wait();
    REQUIRE(subscription.is_closed());
    subscription.close();
  }

  TEST_CASE("upstream_failure") {
    auto operations = std::make_shared<TestMarketDataClient::Queue>();
    auto client = MarketDataClient(
      std::in_place_type<TestMarketDataClient>, operations);
    auto subscription = BboQuoteSubscription(client, parse_ticker("TST.TSX"));
    auto operation = operations->pop();
    auto& query = std::get<
      TestMarketDataClient::QuerySequencedBboQuoteOperation>(*operation);
    query.m_queue.close(std::runtime_error("Query failed."));
    REQUIRE_THROWS_AS(subscription.get_snapshot(), std::runtime_error);
    REQUIRE_THROWS_AS(subscription.pop(), std::runtime_error);
    REQUIRE_FALSE(subscription.is_closed());
  }
}
