#include <Beam/Queues/Queue.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

TEST_SUITE("MarketDataClient") {
  TEST_CASE("query_real_time_with_snapshot_book_quotes_empty") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto book_queue = std::make_shared<Queue<BookQuote>>();
    auto security = Security("SYM", NASDAQ);
    query_real_time_with_snapshot(client, security, book_queue);
    auto operation1 = operations->Pop();
    auto load_operation =
      std::get_if<TestMarketDataClient::LoadSecuritySnapshotOperation>(
        &*operation1);
    REQUIRE(load_operation);
    REQUIRE(load_operation->m_security == security);
    auto snapshot = SecuritySnapshot(security);
    load_operation->m_result.set(snapshot);
    auto operation2 = operations->Pop();
    auto query_operation =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation2);
    REQUIRE(query_operation);
    REQUIRE(query_operation->m_query.GetIndex() == security);
    REQUIRE(query_operation->m_query.GetRange() == Range::RealTime());
    REQUIRE(query_operation->m_query.GetInterruptionPolicy() ==
      InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("query_real_time_with_snapshot_book_quotes") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto book_queue = std::make_shared<Queue<BookQuote>>();
    auto security = Security("SYM", NASDAQ);
    query_real_time_with_snapshot(client, security, book_queue);
    auto operation1 = operations->Pop();
    auto load_operation =
      std::get_if<TestMarketDataClient::LoadSecuritySnapshotOperation>(
        &*operation1);
    auto snapshot = SecuritySnapshot(security);
    snapshot.m_asks.push_back(SequencedBookQuote(
      BookQuote("MP", false, CHIC, Quote(12 * Money::CENT, 222, Side::ASK),
        time_from_string("2021-01-11 15:30:05.000")),
        Beam::Queries::Sequence(5)));
    snapshot.m_bids.push_back(
      SequencedBookQuote(BookQuote("MP", false, PURE, Quote(9 * Money::CENT, 44,
        Side::BID), time_from_string("2021-01-11 15:30:05.000")),
        Beam::Queries::Sequence(7)));
    load_operation->m_result.set(snapshot);
    auto book_quote = book_queue->Pop();
    REQUIRE(book_quote == snapshot.m_asks.front());
    book_quote = book_queue->Pop();
    REQUIRE(book_quote == snapshot.m_bids.front());
    auto operation2 = operations->Pop();
    auto continuation_operation =
      std::get_if<TestMarketDataClient::QueryBookQuoteOperation>(&*operation2);
    REQUIRE(continuation_operation);
    REQUIRE(continuation_operation->m_query.GetIndex() == security);
    REQUIRE(continuation_operation->m_query.GetRange().GetStart() ==
      Beam::Queries::Sequence(8));
    REQUIRE(continuation_operation->m_query.GetRange().GetEnd() ==
      Beam::Queries::Sequence::Last());
    REQUIRE(continuation_operation->m_query.GetSnapshotLimit() ==
      SnapshotLimit::Unlimited());
  }

  TEST_CASE("query_real_time_with_snapshot_bbo") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto quote_queue = std::make_shared<Queue<BboQuote>>();
    auto security = Security("SYM", NASDAQ);
    query_real_time_with_snapshot(client, security, quote_queue);
    auto operation1 = operations->Pop();
    auto* snapshot_operation =
      std::get_if<TestMarketDataClient::QuerySequencedBboQuoteOperation>(
        &*operation1);
    REQUIRE(snapshot_operation);
    REQUIRE(snapshot_operation->m_query.GetIndex() == security);
    auto sequenced_quote = SequencedBboQuote(BboQuote(
      Quote(50 * Money::CENT, 213, Side::BID),
      Quote(55 * Money::CENT, 312, Side::ASK),
      time_from_string("2021-02-25 15:30:05.000")), Beam::Queries::Sequence(3));
    snapshot_operation->m_queue.Push(sequenced_quote);
    auto quote = quote_queue->Pop();
    REQUIRE(quote == sequenced_quote.GetValue());
    auto operation2 = operations->Pop();
    auto continuation_operation =
      std::get_if<TestMarketDataClient::QueryBboQuoteOperation>(&*operation2);
    REQUIRE(continuation_operation);
    REQUIRE(continuation_operation->m_query.GetIndex() == security);
    REQUIRE(continuation_operation->m_query.GetRange().GetStart() ==
      Beam::Queries::Sequence(4));
    REQUIRE(continuation_operation->m_query.GetSnapshotLimit() ==
      SnapshotLimit::Unlimited());
  }

  TEST_CASE("load_security_info") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>();
    auto client = TestMarketDataClient(operations);
    auto security = Security("SYM", NASDAQ);
    auto queue = Queue<optional<SecurityInfo>>();
    Spawn([&] {
      queue.Push(load_security_info(client, security));
    });
    auto operation1 = operations->Pop();
    auto* info_operation =
      std::get_if<TestMarketDataClient::SecurityInfoQueryOperation>(
        &*operation1);
    REQUIRE(info_operation);
    REQUIRE(info_operation->m_query.GetIndex() == security);
    SUBCASE("single_result") {
      auto info_result = SecurityInfo(security, "SYMBOL", "Tech", 100);
      info_operation->m_result.set({info_result});
      auto received_info = queue.Pop();
      REQUIRE(received_info == info_result);
    }
    SUBCASE("empty") {
      info_operation->m_result.set(std::vector<SecurityInfo>());
      auto received_info = queue.Pop();
      REQUIRE(received_info == none);
    }
    SUBCASE("multiple_result") {
      auto info_result = SecurityInfo(security, "SYMBOL", "Tech", 100);
      auto info_result2 =
        SecurityInfo(Security("FOO", NYSE), "LOBMYS", "Hcet", 200);
      info_operation->m_result.set({info_result, info_result2});
      auto received_info = queue.Pop();
      REQUIRE(received_info == info_result);
    }
  }
}
