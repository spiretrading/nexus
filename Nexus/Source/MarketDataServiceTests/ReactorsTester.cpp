#include <future>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/Reactors.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("Reactors") {
  TEST_CASE("make_bbo_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto query = make_real_time_query(parse_ticker("FOO.TSX"));
    auto reactor = make_bbo_quote_reactor(client, query);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == query.get_index());
    REQUIRE(received_query->m_query.get_range() == Range::REAL_TIME);
  }

  TEST_CASE("make_current_bbo_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("FOO.TSX");
    auto reactor = make_current_bbo_quote_reactor(client, ticker);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == ticker);
    REQUIRE(received_query->m_query.get_range() == Range::TOTAL);
    REQUIRE(received_query->m_query.get_snapshot_limit() ==
      SnapshotLimit::from_tail(1));
    REQUIRE(received_query->m_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_real_time_bbo_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("FOO.TSX");
    auto reactor = make_real_time_bbo_quote_reactor(client, ticker);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == ticker);
    REQUIRE(received_query->m_query.get_range() == Range::REAL_TIME);
    REQUIRE(received_query->m_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_book_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto query = make_real_time_query(parse_ticker("FOO.TSX"));
    auto reactor = make_book_quote_reactor(client, query);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == query.get_index());
    REQUIRE(received_query->m_query.get_range() == Range::REAL_TIME);
  }

  TEST_CASE("make_current_book_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("FOO.TSX");
    auto reactor = make_current_book_quote_reactor(client, ticker);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == ticker);
    REQUIRE(received_query->m_query.get_range() == Range::TOTAL);
    REQUIRE(received_query->m_query.get_snapshot_limit() ==
      SnapshotLimit::from_tail(1));
    REQUIRE(received_query->m_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_real_time_book_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("FOO.TSX");
    auto reactor = make_real_time_book_quote_reactor(client, ticker);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == ticker);
    REQUIRE(received_query->m_query.get_range() == Range::REAL_TIME);
    REQUIRE(received_query->m_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_time_and_sales_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto query = make_real_time_query(parse_ticker("FOO.TSX"));
    auto reactor = make_time_and_sales_reactor(client, query);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryTimeAndSaleOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == query.get_index());
    REQUIRE(received_query->m_query.get_range() == Range::REAL_TIME);
  }

  TEST_CASE("make_current_time_and_sales_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("FOO.TSX");
    auto reactor = make_current_time_and_sales_reactor(client, ticker);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryTimeAndSaleOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == ticker);
    REQUIRE(received_query->m_query.get_range() == Range::TOTAL);
    REQUIRE(received_query->m_query.get_snapshot_limit() ==
      SnapshotLimit::from_tail(1));
    REQUIRE(received_query->m_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_real_time_time_and_sales_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto ticker = parse_ticker("FOO.TSX");
    auto reactor = make_real_time_time_and_sales_reactor(client, ticker);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryTimeAndSaleOperation>(&*operation);
    REQUIRE(received_query);
    REQUIRE(received_query->m_query.get_index() == ticker);
    REQUIRE(received_query->m_query.get_range() == Range::REAL_TIME);
    REQUIRE(received_query->m_query.get_interruption_policy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }
}
