#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

TEST_SUITE("MarketDataClient") {
  TEST_CASE("query_real_time_with_snapshot_book_quotes_empty") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto book_queue = std::make_shared<Queue<BookQuote>>();
    auto security = Security("SYM", TSX);
    query_real_time_with_snapshot(client, security, book_queue);
    auto operation1 = operations->pop();
    auto load_operation =
      std::get_if<TestMarketDataClient::LoadSecuritySnapshotOperation>(
        &*operation1);
    REQUIRE(load_operation);
    REQUIRE(load_operation->m_security == security);
    auto snapshot = SecuritySnapshot(security);
    load_operation->m_result.set(snapshot);
    auto operation2 = operations->pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation2);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.get_index() == security);
    REQUIRE(query_operation->m_query.get_range() == Range::REAL_TIME);
    REQUIRE(query_operation->m_query.get_interruption_policy() ==
      InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("query_real_time_with_snapshot_book_quotes") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto book_queue = std::make_shared<Queue<BookQuote>>();
    auto security = Security("SYM", TSX);
    query_real_time_with_snapshot(client, security, book_queue);
    auto operation1 = operations->pop();
    auto load_operation =
      std::get_if<TestMarketDataClient::LoadSecuritySnapshotOperation>(
        &*operation1);
    auto snapshot = SecuritySnapshot(security);
    snapshot.m_asks.push_back(SequencedBookQuote(
      BookQuote("MP", false, CHIC, make_ask(12 * Money::CENT, 222),
        time_from_string("2021-01-11 15:30:05.000")), Beam::Sequence(5)));
    snapshot.m_bids.push_back(SequencedBookQuote(
      BookQuote("MP", false, PURE, make_bid(9 * Money::CENT, 44),
        time_from_string("2021-01-11 15:30:05.000")), Beam::Sequence(7)));
    load_operation->m_result.set(snapshot);
    auto book_quote = book_queue->pop();
    REQUIRE(book_quote == snapshot.m_asks.front());
    book_quote = book_queue->pop();
    REQUIRE(book_quote == snapshot.m_bids.front());
    auto operation2 = operations->pop();
    auto continuation_operation =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation2);
    REQUIRE(continuation_operation);
    REQUIRE(continuation_operation->m_query.get_index() == security);
    REQUIRE(continuation_operation->m_query.get_range().get_start() ==
      Beam::Sequence(8));
    REQUIRE(continuation_operation->m_query.get_range().get_end() ==
      Beam::Sequence::LAST);
    REQUIRE(continuation_operation->m_query.get_snapshot_limit() ==
      SnapshotLimit::UNLIMITED);
  }

  TEST_CASE("query_real_time_with_snapshot_bbo") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto quote_queue = std::make_shared<Queue<BboQuote>>();
    auto security = Security("SYM", TSX);
    query_real_time_with_snapshot(client, security, quote_queue);
    auto operation1 = operations->pop();
    auto* snapshot_operation =
      std::get_if<TestMarketDataClient::QuerySequencedBboQuoteOperation>(
        &*operation1);
    REQUIRE(snapshot_operation);
    REQUIRE(snapshot_operation->m_query.get_index() == security);
    auto sequenced_quote = SequencedBboQuote(BboQuote(
      make_bid(50 * Money::CENT, 213), make_ask(55 * Money::CENT, 312),
      time_from_string("2021-02-25 15:30:05.000")), Beam::Sequence(3));
    snapshot_operation->m_queue.push(sequenced_quote);
    auto quote = quote_queue->pop();
    REQUIRE(quote == *sequenced_quote);
    auto operation2 = operations->pop();
    auto continuation_operation =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation2);
    REQUIRE(continuation_operation);
    REQUIRE(continuation_operation->m_query.get_index() == security);
    REQUIRE(continuation_operation->m_query.get_range().get_start() ==
      Beam::Sequence(4));
    REQUIRE(continuation_operation->m_query.get_snapshot_limit() ==
      SnapshotLimit::UNLIMITED);
  }

  TEST_CASE("load_security_info") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("SYM", TSX);
    auto queue = Queue<optional<SecurityInfo>>();
    spawn([&] {
      queue.push(load_security_info(client, security));
    });
    auto operation1 = operations->pop();
    auto* info_operation =
      std::get_if<TestMarketDataClient::SecurityInfoQueryOperation>(
        &*operation1);
    REQUIRE(info_operation);
    REQUIRE(info_operation->m_query.get_index() == security);
    SUBCASE("single_result") {
      auto info_result = SecurityInfo(security, "SYMBOL", "Tech", 100);
      info_operation->m_result.set({info_result});
      auto received_info = queue.pop();
      REQUIRE(received_info == info_result);
    }
    SUBCASE("empty") {
      info_operation->m_result.set(std::vector<SecurityInfo>());
      auto received_info = queue.pop();
      REQUIRE(received_info == none);
    }
    SUBCASE("multiple_result") {
      auto info_result = SecurityInfo(security, "SYMBOL", "Tech", 100);
      auto info_result2 =
        SecurityInfo(Security("FOO", ASX), "LOBMYS", "Hcet", 200);
      info_operation->m_result.set({info_result, info_result2});
      auto received_info = queue.pop();
      REQUIRE(received_info == info_result);
    }
  }
}
