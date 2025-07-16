#include <future>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/Reactors.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

TEST_SUITE("Reactors") {
  TEST_CASE("make_bbo_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto query = MakeRealTimeQuery(Security("FOO", TSX));
    auto reactor = make_bbo_quote_reactor(client, query);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == query.GetIndex());
    REQUIRE(received_query->m_query.GetRange() == Range::RealTime());
  }

  TEST_CASE("make_current_bbo_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("FOO", TSX);
    auto reactor = make_current_bbo_quote_reactor(client, security);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == security);
    REQUIRE(received_query->m_query.GetRange() == Range::Total());
    REQUIRE(
      received_query->m_query.GetSnapshotLimit() == SnapshotLimit::FromTail(1));
    REQUIRE(received_query->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_real_time_bbo_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("FOO", TSX);
    auto reactor = make_real_time_bbo_quote_reactor(client, security);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == security);
    REQUIRE(received_query->m_query.GetRange() == Range::RealTime());
    REQUIRE(received_query->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_book_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto query = MakeRealTimeQuery(Security("FOO", TSX));
    auto reactor = make_book_quote_reactor(client, query);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == query.GetIndex());
    REQUIRE(received_query->m_query.GetRange() == Range::RealTime());
  }

  TEST_CASE("make_current_book_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("FOO", TSX);
    auto reactor = make_current_book_quote_reactor(client, security);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == security);
    REQUIRE(received_query->m_query.GetRange() == Range::Total());
    REQUIRE(
      received_query->m_query.GetSnapshotLimit() == SnapshotLimit::FromTail(1));
    REQUIRE(received_query->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_real_time_book_quote_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("FOO", TSX);
    auto reactor = make_real_time_book_quote_reactor(client, security);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == security);
    REQUIRE(received_query->m_query.GetRange() == Range::RealTime());
    REQUIRE(received_query->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_time_and_sales_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto query = MakeRealTimeQuery(Security("FOO", TSX));
    auto reactor = make_time_and_sales_reactor(client, query);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryTimeAndSaleOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == query.GetIndex());
    REQUIRE(received_query->m_query.GetRange() == Range::RealTime());
  }

  TEST_CASE("make_current_time_and_sales_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("FOO", TSX);
    auto reactor = make_current_time_and_sales_reactor(client, security);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryTimeAndSaleOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == security);
    REQUIRE(received_query->m_query.GetRange() == Range::Total());
    REQUIRE(
      received_query->m_query.GetSnapshotLimit() == SnapshotLimit::FromTail(1));
    REQUIRE(received_query->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }

  TEST_CASE("make_real_time_time_and_sales_reactor") {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("FOO", TSX);
    auto reactor = make_real_time_time_and_sales_reactor(client, security);
    REQUIRE(reactor.commit(0) == State::NONE);
    auto operation = operations->Pop();
    auto received_query =
      std::get_if<TestMarketDataClient::QueryTimeAndSaleOperation>(&*operation);
    REQUIRE(received_query != nullptr);
    REQUIRE(received_query->m_query.GetIndex() == security);
    REQUIRE(received_query->m_query.GetRange() == Range::RealTime());
    REQUIRE(received_query->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::IGNORE_CONTINUE);
  }
}
