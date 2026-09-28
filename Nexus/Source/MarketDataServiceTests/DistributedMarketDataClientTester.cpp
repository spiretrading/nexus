#include <atomic>
#include <future>
#include <thread>
#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataService/DataStoreMarketDataClient.hpp"
#include "Nexus/MarketDataService/DistributedMarketDataClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Countries;
using namespace Nexus::Tests;
using namespace Nexus::Venues;

namespace {
  template<typename T>
  auto require_operation(
      std::shared_ptr<TestMarketDataClient::Operation> operation) {
    auto unwrapped_operation = std::get_if<T>(operation.get());
    REQUIRE(unwrapped_operation);
    return std::shared_ptr<T>(operation, unwrapped_operation);
  }

  auto make_operations_queues() {
    auto operations = ScopeMap<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>>(nullptr);
    operations.set(TSX, std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>());
    operations.set(AU, std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>());
    return operations;
  }

  auto make_market_data_clients(const ScopeMap<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>>& operations) {
    auto clients = ScopeMap<std::shared_ptr<MarketDataClient>>(nullptr);
    clients.set(TSX, std::make_shared<MarketDataClient>(
      std::in_place_type<TestMarketDataClient>, operations.get(TSX)));
    clients.set(AU, std::make_shared<MarketDataClient>(
      std::in_place_type<TestMarketDataClient>, operations.get(AU)));
    return clients;
  }

  struct DiscoveryFixture {
    ScopeMap<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>> m_operations;
    ScopeMap<std::shared_ptr<MarketDataClient>> m_clients;
    QueueWriterPublisher<ServiceUpdate> m_services;
    std::shared_ptr<TriggerTimer> m_timer;
    std::atomic_bool m_fail_connection;
    Queue<Scope> m_attempts;
    std::unique_ptr<DistributedMarketDataClient<
      std::shared_ptr<TriggerTimer>>> m_client;

    DiscoveryFixture()
      : DiscoveryFixture(std::vector<ServiceEntry>()) {}

    explicit DiscoveryFixture(std::vector<ServiceEntry> services)
        : m_operations(make_operations_queues()),
          m_clients(make_market_data_clients(m_operations)),
          m_timer(std::make_shared<TriggerTimer>()),
          m_fail_connection(false) {
      m_client = std::make_unique<
        DistributedMarketDataClient<std::shared_ptr<TriggerTimer>>>(
        [&] (ScopedQueueWriter<ServiceUpdate> queue) {
          for(auto& service : services) {
            queue.push(ServiceUpdate::add(service));
          }
          m_services.monitor(std::move(queue));
        }, [] (const auto& service) {
          if(service.get_name() == "tsx") {
            return Scope(TSX);
          }
          return Scope(AU);
        }, [=, this] (const Scope& scope) {
          m_attempts.push(scope);
          if(m_fail_connection && scope == AU) {
            throw ConnectException();
          }
          return m_clients.get(scope);
        }, m_timer);
      flush_pending_routines();
    }

    void update(const ServiceUpdate& update) {
      m_services.push(update);
      flush_pending_routines();
    }

    void retry() {
      m_timer->trigger();
      flush_pending_routines();
    }
  };

  struct Fixture {
    ScopeMap<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>> m_operations;
    DistributedMarketDataClient<Timer> m_client;

    Fixture()
      : m_operations(make_operations_queues()),
        m_client(make_market_data_clients(m_operations)) {}
  };
}

TEST_SUITE("DistributedMarketDataClient") {
  TEST_CASE("discover_scopes") {
    auto tsx = ServiceEntry("tsx", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT);
    auto fixture = DiscoveryFixture({tsx});
    REQUIRE(fixture.m_attempts.pop() == TSX);
    auto quotes = std::make_shared<Queue<BboQuote>>();
    fixture.m_client->query(
      make_real_time_query(parse_ticker("ABC.TSX")), quotes);
    auto subscription = require_operation<
      TestMarketDataClient::QueryBboQuoteOperation>(
        fixture.m_operations.get(TSX)->pop());
    auto australian = ServiceEntry("au", JsonObject(), 2,
      DirectoryEntry::ROOT_ACCOUNT);
    fixture.update(ServiceUpdate::add(australian));
    REQUIRE(fixture.m_attempts.pop() == AU);
    auto australian_quotes = std::make_shared<Queue<BboQuote>>();
    fixture.m_client->query(make_real_time_query(parse_ticker("S32.ASX")),
      australian_quotes);
    auto australian_subscription = require_operation<
      TestMarketDataClient::QueryBboQuoteOperation>(
        fixture.m_operations.get(AU)->pop());
    fixture.update(ServiceUpdate::remove(tsx));
    fixture.update(ServiceUpdate::add(tsx));
    fixture.update(ServiceUpdate::add(australian));
    fixture.update(ServiceUpdate::add(ServiceEntry("au", JsonObject(), 3,
      DirectoryEntry::ROOT_ACCOUNT)));
    fixture.retry();
    REQUIRE(!fixture.m_attempts.try_pop());
    auto quote = BboQuote(make_bid(Money::ONE, 100),
      make_ask(2 * Money::ONE, 200), time_from_string("2026-09-28 12:00:00"));
    subscription->m_queue.push(quote);
    australian_subscription->m_queue.push(quote);
    REQUIRE(quotes->pop() == quote);
    REQUIRE(australian_quotes->pop() == quote);
  }

  TEST_CASE("discover_scopes_empty_snapshot") {
    auto fixture = DiscoveryFixture();
    REQUIRE(!fixture.m_attempts.try_pop());
    auto result = Async<void>();
    auto request = RoutineHandler(spawn([&] {
      try {
        fixture.m_client->query(make_real_time_query(parse_ticker("ABC.TSX")),
          std::make_shared<Queue<BboQuote>>());
        result.get_eval().set();
      } catch(...) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(result.get_state() == BaseAsync::State::PENDING);
    fixture.update(ServiceUpdate::add(ServiceEntry("tsx", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT)));
    REQUIRE(fixture.m_attempts.pop() == TSX);
    result.get();
    require_operation<TestMarketDataClient::QueryBboQuoteOperation>(
      fixture.m_operations.get(TSX)->pop());
  }

  TEST_CASE("query_ticker_info_waits_for_scope") {
    auto fixture = DiscoveryFixture();
    fixture.update(ServiceUpdate::add(ServiceEntry("tsx", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT)));
    auto ticker = parse_ticker("S32.ASX");
    auto result = Async<std::vector<TickerInfo>>();
    auto request = RoutineHandler(spawn([&] {
      try {
        result.get_eval().set(
          fixture.m_client->query(make_ticker_info_query(ticker)));
      } catch(...) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(result.get_state() == BaseAsync::State::PENDING);
    REQUIRE(!fixture.m_operations.get(TSX)->try_pop());
    fixture.m_fail_connection = true;
    fixture.update(ServiceUpdate::add(ServiceEntry("au", JsonObject(), 2,
      DirectoryEntry::ROOT_ACCOUNT)));
    REQUIRE(result.get_state() == BaseAsync::State::PENDING);
    fixture.m_fail_connection = false;
    fixture.retry();
    auto operation = require_operation<TestMarketDataClient::
      TickerInfoQueryOperation>(fixture.m_operations.get(AU)->pop());
    auto info = TickerInfo(ticker, "South32", "Materials", 100);
    operation->m_result.set({info});
    REQUIRE(result.get() == std::vector{info});
  }

  TEST_CASE("pending_query_close") {
    auto fixture = DiscoveryFixture();
    auto query = std::function<void ()>();

    SUBCASE("ticker_info") {
      query = [&] {
        fixture.m_client->query(
          make_ticker_info_query(parse_ticker("ABC.TSX")));
      };
    }

    SUBCASE("prefix") {
      query = [&] {
        fixture.m_client->load_ticker_info_from_prefix("ABC");
      };
    }

    SUBCASE("imbalance") {
      query = [&] {
        fixture.m_client->query(make_real_time_query(TSX),
          std::make_shared<Queue<OrderImbalance>>());
      };
    }

    auto result = Async<void>();
    auto request = RoutineHandler(spawn([&] {
      try {
        query();
        result.get_eval().set();
      } catch(...) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(result.get_state() == BaseAsync::State::PENDING);
    fixture.m_client->close();
    REQUIRE_THROWS_AS(result.get(), EndOfFileException);
  }

  TEST_CASE("pending_query_subscription_close") {
    auto fixture = DiscoveryFixture();
    auto result = Async<std::vector<TickerInfo>>();
    auto request = RoutineHandler(spawn([&] {
      try {
        result.get_eval().set(fixture.m_client->query(
          make_ticker_info_query(parse_ticker("ABC.TSX"))));
      } catch(...) {
        result.get_eval().set_exception(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(result.get_state() == BaseAsync::State::PENDING);
    fixture.m_services.close();
    REQUIRE_THROWS_AS(result.get(), PipeBrokenException);
  }

  TEST_CASE("discover_scopes_connection_failure") {
    auto fixture = DiscoveryFixture();
    fixture.m_fail_connection = true;
    fixture.update(ServiceUpdate::add(ServiceEntry("au", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT)));
    REQUIRE(fixture.m_attempts.pop() == AU);
    fixture.update(ServiceUpdate::add(ServiceEntry("tsx", JsonObject(), 2,
      DirectoryEntry::ROOT_ACCOUNT)));
    REQUIRE(fixture.m_attempts.pop() == TSX);
    fixture.retry();
    REQUIRE(fixture.m_attempts.pop() == AU);
    REQUIRE(!fixture.m_attempts.try_pop());
    fixture.m_fail_connection = false;
    fixture.retry();
    REQUIRE(fixture.m_attempts.pop() == AU);
    fixture.m_client->query(make_real_time_query(parse_ticker("S32.ASX")),
      std::make_shared<Queue<BboQuote>>());
    require_operation<TestMarketDataClient::QueryBboQuoteOperation>(
      fixture.m_operations.get(AU)->pop());
    fixture.retry();
    REQUIRE(!fixture.m_attempts.try_pop());
  }

  TEST_CASE("discover_scopes_withdrawn_registration") {
    auto fixture = DiscoveryFixture();
    fixture.m_fail_connection = true;
    auto first = ServiceEntry("au", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT);
    auto second = ServiceEntry("au", JsonObject(), 2,
      DirectoryEntry::ROOT_ACCOUNT);
    fixture.update(ServiceUpdate::add(first));
    fixture.update(ServiceUpdate::add(second));
    REQUIRE(fixture.m_attempts.pop() == AU);
    REQUIRE(fixture.m_attempts.pop() == AU);
    fixture.retry();
    REQUIRE(fixture.m_attempts.pop() == AU);
    REQUIRE(!fixture.m_attempts.try_pop());
    fixture.update(ServiceUpdate::remove(first));
    fixture.retry();
    REQUIRE(fixture.m_attempts.pop() == AU);
    fixture.update(ServiceUpdate::remove(second));
    fixture.retry();
    REQUIRE(!fixture.m_attempts.try_pop());
    fixture.m_fail_connection = false;
    fixture.update(ServiceUpdate::add(second));
    REQUIRE(fixture.m_attempts.pop() == AU);
  }

  TEST_CASE("discover_scopes_replaced_registration") {
    auto fixture = DiscoveryFixture();
    fixture.m_fail_connection = true;
    auto original = ServiceEntry("tsx", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT);
    fixture.update(ServiceUpdate::add(original));
    REQUIRE(fixture.m_attempts.pop() == TSX);
    auto replacement = ServiceEntry("au", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT);
    fixture.update(ServiceUpdate::add(replacement));
    REQUIRE(fixture.m_attempts.pop() == AU);
    fixture.update(ServiceUpdate::remove(original));
    fixture.retry();
    REQUIRE(fixture.m_attempts.pop() == AU);
    REQUIRE(!fixture.m_attempts.try_pop());
    fixture.update(ServiceUpdate::remove(replacement));
    fixture.retry();
    REQUIRE(!fixture.m_attempts.try_pop());
    fixture.m_client->query(make_real_time_query(parse_ticker("ABC.TSX")),
      std::make_shared<Queue<BboQuote>>());
    require_operation<TestMarketDataClient::QueryBboQuoteOperation>(
      fixture.m_operations.get(TSX)->pop());
  }

  TEST_CASE("discover_scopes_close") {
    auto fixture = DiscoveryFixture();
    fixture.m_fail_connection = true;
    fixture.update(ServiceUpdate::add(ServiceEntry("au", JsonObject(), 1,
      DirectoryEntry::ROOT_ACCOUNT)));
    REQUIRE(fixture.m_attempts.pop() == AU);

    SUBCASE("client") {
      fixture.m_client->close();
    }

    SUBCASE("subscription") {
      fixture.m_services.close();
      flush_pending_routines();
    }

    fixture.retry();
    fixture.update(ServiceUpdate::add(ServiceEntry("tsx", JsonObject(), 2,
      DirectoryEntry::ROOT_ACCOUNT)));
    REQUIRE(!fixture.m_attempts.try_pop());
  }

  TEST_CASE("query_sequenced_order_imbalances") {
    auto fixture = Fixture();
    auto imbalances = std::make_shared<Queue<SequencedOrderImbalance>>();
    auto query = VenueQuery();

    SUBCASE("exact") {
      query.set_index(TSX);
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
          operations->pop());
      auto test_imbalance = SequencedValue(
        OrderImbalance(parse_ticker("ABC.TSX"), Side::BID, 100, Money::ONE,
          time_from_string("2024-06-12 13:05:12:00")), Beam::Sequence(100));
      received_query->m_queue.push(test_imbalance);
      auto received_imbalance = imbalances->pop();
      REQUIRE(received_imbalance == test_imbalance);
    }

    SUBCASE("parent") {
      query.set_index(ASX);
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
          operations->pop());
      auto test_imbalance = SequencedValue(
        OrderImbalance(parse_ticker("S32.ASX"), Side::ASK, 200, 3 * Money::ONE,
          time_from_string("2025-02-18 17:23:30:12")), Beam::Sequence(200));
      received_query->m_queue.push(test_imbalance);
      auto received_imbalance = imbalances->pop();
      REQUIRE(received_imbalance == test_imbalance);
    }

    SUBCASE("unavailable") {
      query.set_index(TSXV);
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, imbalances);
      REQUIRE_THROWS_AS(imbalances->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_order_imbalances") {
    auto fixture = Fixture();
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto query = VenueQuery();

    SUBCASE("exact") {
      query.set_index(TSX);
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryOrderImbalanceOperation>(operations->pop());
      auto test_imbalance = OrderImbalance(parse_ticker("ABC.TSX"), Side::BID,
        100, Money::ONE, time_from_string("2024-06-12 13:05:12:00"));
      received_query->m_queue.push(test_imbalance);
      auto received_imbalance = imbalances->pop();
      REQUIRE(received_imbalance == test_imbalance);
    }

    SUBCASE("parent") {
      query.set_index(ASX);
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryOrderImbalanceOperation>(operations->pop());
      auto test_imbalance = OrderImbalance(parse_ticker("S32.ASX"), Side::ASK,
        200, 3 * Money::ONE, time_from_string("2025-02-18 17:23:30:12"));
      received_query->m_queue.push(test_imbalance);
      auto received_imbalance = imbalances->pop();
      REQUIRE(received_imbalance == test_imbalance);
    }

    SUBCASE("unavailable") {
      query.set_index(TSXV);
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, imbalances);
      REQUIRE_THROWS_AS(imbalances->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_bbo_quotes") {
    auto fixture = Fixture();
    auto bbo_quotes = std::make_shared<Queue<SequencedBboQuote>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBboQuoteOperation>(
          operations->pop());
      auto test_bbo = SequencedValue(BboQuote(
        make_bid(10 * Money::ONE, 100), make_ask(11 * Money::ONE, 100),
        time_from_string("2024-06-12 13:05:12:00")), Beam::Sequence(100));
      received_query->m_queue.push(test_bbo);
      auto received_bbo = bbo_quotes->pop();
      REQUIRE(received_bbo == test_bbo);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBboQuoteOperation>(
          operations->pop());
      auto test_bbo = SequencedValue(BboQuote(
        make_bid(20 * Money::ONE, 200), make_ask(21 * Money::ONE, 200),
        time_from_string("2025-02-18 17:23:30:12")), Beam::Sequence(200));
      received_query->m_queue.push(test_bbo);
      auto received_bbo = bbo_quotes->pop();
      REQUIRE(received_bbo == test_bbo);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      REQUIRE_THROWS_AS(bbo_quotes->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_bbo_quotes") {
    auto fixture = Fixture();
    auto bbo_quotes = std::make_shared<Queue<BboQuote>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBboQuoteOperation>(operations->pop());
      auto test_bbo =
        BboQuote(make_bid(10 * Money::ONE, 100), make_ask(11 * Money::ONE, 100),
          time_from_string("2024-06-12 13:05:12:00"));
      received_query->m_queue.push(test_bbo);
      auto received_bbo = bbo_quotes->pop();
      REQUIRE(received_bbo == test_bbo);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBboQuoteOperation>(operations->pop());
      auto test_bbo =
        BboQuote(make_bid(20 * Money::ONE, 200), make_ask(21 * Money::ONE, 200),
          time_from_string("2025-02-18 17:23:30:12"));
      received_query->m_queue.push(test_bbo);
      auto received_bbo = bbo_quotes->pop();
      REQUIRE(received_bbo == test_bbo);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      REQUIRE_THROWS_AS(bbo_quotes->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_book_quotes") {
    auto fixture = Fixture();
    auto book_quotes = std::make_shared<Queue<SequencedBookQuote>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBookQuoteOperation>(
          operations->pop());
      auto test_book_quote = SequencedValue(
        BookQuote("MMID12", true, TSX, make_bid(10 * Money::ONE, 100),
          time_from_string("2024-06-12 13:05:12:00")), Beam::Sequence(100));
      received_query->m_queue.push(test_book_quote);
      auto received_book_quote = book_quotes->pop();
      REQUIRE(received_book_quote == test_book_quote);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBookQuoteOperation>(
          operations->pop());
      auto test_book_quote = SequencedValue(
        BookQuote("MMID5", false, ASX, make_ask(20 * Money::ONE, 200),
          time_from_string("2025-02-18 17:23:30:12")), Beam::Sequence(200));
      received_query->m_queue.push(test_book_quote);
      auto received_book_quote = book_quotes->pop();
      REQUIRE(received_book_quote == test_book_quote);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      REQUIRE_THROWS_AS(book_quotes->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_book_quotes") {
    auto fixture = Fixture();
    auto book_quotes = std::make_shared<Queue<BookQuote>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBookQuoteOperation>(operations->pop());
      auto test_book_quote = SequencedValue(
        BookQuote("MMID12", true, TSX, make_bid(10 * Money::ONE, 100),
          time_from_string("2024-06-12 13:05:12:00")), Beam::Sequence(100));
      received_query->m_queue.push(test_book_quote);
      auto received_book_quote = book_quotes->pop();
      REQUIRE(received_book_quote == test_book_quote);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBookQuoteOperation>(operations->pop());
      auto test_book_quote = SequencedValue(
        BookQuote("MMID5", false, ASX, make_ask(20 * Money::ONE, 200),
          time_from_string("2025-02-18 17:23:30:12")),
          Beam::Sequence(200));
      received_query->m_queue.push(test_book_quote);
      auto received_book_quote = book_quotes->pop();
      REQUIRE(received_book_quote == test_book_quote);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      REQUIRE_THROWS_AS(book_quotes->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_ticker_statuses") {
    auto fixture = Fixture();
    auto ticker_statuses = std::make_shared<Queue<SequencedTickerStatus>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, ticker_statuses);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedTickerStatusOperation>(
          operations->pop());
      auto test_status = SequencedValue(
        TickerStatus(TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
          time_from_string("2024-07-09 09:30:00")), Beam::Sequence(100));
      received_query->m_queue.push(test_status);
      auto received_status = ticker_statuses->pop();
      REQUIRE(received_status == test_status);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, ticker_statuses);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedTickerStatusOperation>(
          operations->pop());
      auto test_status = SequencedValue(
        TickerStatus(ASX, "PreOpen", TickerStatus::Flag::IS_ACCEPTING_ORDERS,
          time_from_string("2025-02-18 07:00:00")), Beam::Sequence(200));
      received_query->m_queue.push(test_status);
      auto received_status = ticker_statuses->pop();
      REQUIRE(received_status == test_status);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, ticker_statuses);
      REQUIRE_THROWS_AS(ticker_statuses->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_ticker_statuses") {
    auto fixture = Fixture();
    auto ticker_statuses = std::make_shared<Queue<TickerStatus>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, ticker_statuses);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryTickerStatusOperation>(operations->pop());
      auto test_status =
        TickerStatus(TSX, "Authorized", TickerStatus::Flag::IS_CONTINUOUS,
          time_from_string("2024-07-09 09:30:00"));
      received_query->m_queue.push(test_status);
      auto received_status = ticker_statuses->pop();
      REQUIRE(received_status == test_status);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, ticker_statuses);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryTickerStatusOperation>(operations->pop());
      auto test_status =
        TickerStatus(ASX, "PreOpen", TickerStatus::Flag::IS_ACCEPTING_ORDERS,
          time_from_string("2025-02-18 07:00:00"));
      received_query->m_queue.push(test_status);
      auto received_status = ticker_statuses->pop();
      REQUIRE(received_status == test_status);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, ticker_statuses);
      REQUIRE_THROWS_AS(ticker_statuses->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_time_and_sales") {
    auto fixture = Fixture();
    auto time_and_sales = std::make_shared<Queue<SequencedTimeAndSale>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedTimeAndSaleOperation>(
          operations->pop());
      auto test_time_and_sale = SequencedValue(TimeAndSale(
        time_from_string("2024-07-09 12:00:00.123"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "TSX", "B1", "S1"), Beam::Sequence(100));
      received_query->m_queue.push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedTimeAndSaleOperation>(
          operations->pop());
      auto test_time_and_sale = SequencedValue(TimeAndSale(
        time_from_string("2025-02-18 17:23:30.12"), 150 * Money::ONE, 200,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "ASX", "B52", "S46"), Beam::Sequence(200));
      received_query->m_queue.push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, time_and_sales);
      REQUIRE_THROWS_AS(time_and_sales->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_time_and_sales") {
    auto fixture = Fixture();
    auto time_and_sales = std::make_shared<Queue<TimeAndSale>>();
    auto query = TickerQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryTimeAndSaleOperation>(operations->pop());
      auto test_time_and_sale = TimeAndSale(
        time_from_string("2024-07-09 12:00:00.123"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "TSX", "B11", "S76");
      received_query->m_queue.push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }

    SUBCASE("parent") {
      query.set_index(parse_ticker("S32.ASX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(ASX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryTimeAndSaleOperation>(operations->pop());
      auto test_time_and_sale = TimeAndSale(
        time_from_string("2025-02-18 17:23:30.12"), 150 * Money::ONE, 200,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "ASX", "B4", "S99");
      received_query->m_queue.push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }

    SUBCASE("unavailable") {
      query.set_index(parse_ticker("BHP.TSXV"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, time_and_sales);
      REQUIRE_THROWS_AS(time_and_sales->pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_ticker_info") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("ABC.TSX");

    SUBCASE("exact") {}

    SUBCASE("parent") {
      ticker = parse_ticker("S32.ASX");
    }

    auto query = TickerInfoQuery();
    query.set_index(ticker);
    auto operations = fixture.m_operations.get(ticker);
    auto result = std::async(std::launch::async, [&] {
      return fixture.m_client.query(query);
    });
    auto received_query = require_operation<
      TestMarketDataClient::TickerInfoQueryOperation>(operations->pop());
    REQUIRE(received_query->m_query.get_index() == ticker);
    auto test_ticker_info = TickerInfo();
    test_ticker_info.m_ticker = ticker;
    test_ticker_info.m_name = "Alphabet Inc.";
    test_ticker_info.m_sector = "Technology";
    test_ticker_info.m_board_lot = 100;
    received_query->m_result.set({test_ticker_info});
    auto received_ticker_info = result.get();
    REQUIRE(received_ticker_info.size() == 1);
    REQUIRE(received_ticker_info.front() == test_ticker_info);
  }

  TEST_CASE("query_ticker_info_across_scopes") {
    auto canadian_store = LocalHistoricalDataStore();
    auto australian_store = LocalHistoricalDataStore();
    auto combined_store = LocalHistoricalDataStore();
    for(auto symbol : {"ABC.TSX", "XYZ.TSX"}) {
      auto info = TickerInfo();
      info.m_ticker = parse_ticker(symbol);
      canadian_store.store(info);
      combined_store.store(info);
    }
    for(auto symbol : {"ABC.TSX", "BHP.ASX", "S32.ASX"}) {
      auto info = TickerInfo();
      info.m_ticker = parse_ticker(symbol);
      australian_store.store(info);
      combined_store.store(info);
    }
    auto clients = ScopeMap<std::shared_ptr<MarketDataClient>>(nullptr);
    clients.set(TSX, std::make_shared<MarketDataClient>(std::in_place_type<
      DataStoreMarketDataClient<LocalHistoricalDataStore*>>, &canadian_store));
    clients.set(AU, std::make_shared<MarketDataClient>(
      std::in_place_type<DataStoreMarketDataClient<LocalHistoricalDataStore*>>,
      &australian_store));
    auto client = DistributedMarketDataClient(clients);
    auto query = TickerInfoQuery();
    query.set_index(Scope::GLOBAL);
    for(auto limit : {SnapshotLimit::from_head(2), SnapshotLimit::from_tail(2),
        SnapshotLimit::UNLIMITED, SnapshotLimit::NONE}) {
      query.set_snapshot_limit(limit);
      for(auto offset : {0, 1, 10}) {
        query.set_offset(offset);
        for(auto anchor : {optional<Ticker>(),
            optional<Ticker>(parse_ticker("S32.ASX"))}) {
          query.set_anchor(anchor);
          REQUIRE(
            client.query(query) == combined_store.load_ticker_info(query));
        }
      }
    }
  }

  TEST_CASE("load_snapshot") {
    auto fixture = Fixture();

    SUBCASE("exact") {
      auto ticker = parse_ticker("ABC.TSX");
      auto operations = fixture.m_operations.get(TSX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_snapshot(ticker);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadTickerSnapshotOperation>(operations->pop());
      REQUIRE(received_operation->m_ticker == ticker);
      auto test_snapshot = TickerSnapshot();
      test_snapshot.m_bbo_quote = SequencedValue(
        BboQuote(make_bid(10 * Money::ONE, 100), make_ask(11 * Money::ONE, 100),
          time_from_string("2024-06-12 13:05:12:00")), Beam::Sequence(100));
      received_operation->m_result.set(test_snapshot);
      auto received_snapshot = result.get();
      REQUIRE(received_snapshot.m_bbo_quote == test_snapshot.m_bbo_quote);
    }

    SUBCASE("parent") {
      auto ticker = parse_ticker("S32.ASX");
      auto operations = fixture.m_operations.get(ASX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_snapshot(ticker);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadTickerSnapshotOperation>(operations->pop());
      REQUIRE(received_operation->m_ticker == ticker);
      auto test_snapshot = TickerSnapshot();
      test_snapshot.m_bbo_quote = SequencedValue(
        BboQuote(make_bid(20 * Money::ONE, 200), make_ask(21 * Money::ONE, 200),
          time_from_string("2025-02-18 17:23:30:12")), Beam::Sequence(200));
      received_operation->m_result.set(test_snapshot);
      auto received_snapshot = result.get();
      REQUIRE(received_snapshot.m_bbo_quote == test_snapshot.m_bbo_quote);
    }

    SUBCASE("unavailable") {
      auto ticker = parse_ticker("BHP.TSXV");
      auto snapshot = fixture.m_client.load_snapshot(ticker);
      REQUIRE(snapshot.m_bbo_quote == SequencedBboQuote());
      REQUIRE(snapshot.m_asks.empty());
      REQUIRE(snapshot.m_bids.empty());
    }
  }

  TEST_CASE("load_session_technicals") {
    auto fixture = Fixture();

    SUBCASE("exact") {
      auto ticker = parse_ticker("ABC.TSX");
      auto operations = fixture.m_operations.get(TSX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_session_technicals(ticker);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSessionTechnicalsOperation>(
          operations->pop());
      REQUIRE(received_operation->m_ticker == ticker);
      auto technicals = SessionTechnicals();
      technicals.m_open = 2 * Money::ONE;
      technicals.m_previous_close = Money::ONE + Money::CENT;
      technicals.m_high = 2 * Money::ONE;
      technicals.m_low = Money::CENT;
      technicals.m_volume = Quantity(100);
      auto test_technicals =
        SequencedSessionTechnicals(technicals, Beam::Sequence(5));
      received_operation->m_result.set(test_technicals);
      auto received_technicals = result.get();
      test_json_equality(received_technicals, test_technicals);
    }

    SUBCASE("parent") {
      auto ticker = parse_ticker("S32.ASX");
      auto operations = fixture.m_operations.get(ASX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_session_technicals(ticker);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSessionTechnicalsOperation>(
          operations->pop());
      REQUIRE(received_operation->m_ticker == ticker);
      auto technicals = SessionTechnicals();
      technicals.m_open = 152 * Money::ONE;
      technicals.m_previous_close = 151 * Money::ONE;
      technicals.m_high = 152 * Money::ONE;
      technicals.m_low = 148 * Money::ONE;
      technicals.m_volume = Quantity(10000);
      auto test_technicals =
        SequencedSessionTechnicals(technicals, Beam::Sequence(5));
      received_operation->m_result.set(test_technicals);
      auto received_technicals = result.get();
      test_json_equality(received_technicals, test_technicals);
    }

    SUBCASE("unavailable") {
      auto ticker = parse_ticker("BHP.TSXV");
      auto technicals = fixture.m_client.load_session_technicals(ticker);
      test_json_equality(technicals, SequencedSessionTechnicals());
    }
  }

  TEST_CASE("load_ticker_info_from_prefix") {
    auto fixture = Fixture();
    auto prefix = "A";
    auto tsx_ticker_info = TickerInfo();
    tsx_ticker_info.m_ticker = parse_ticker("ABC.TSX");
    tsx_ticker_info.m_name = "Alphabet Inc. Class C";
    auto au_ticker_info = TickerInfo();
    au_ticker_info.m_ticker = parse_ticker("S32.ASX");
    au_ticker_info.m_name = "S32 Inc.";
    auto tsx_handler = std::thread([&] {
      auto operations = fixture.m_operations.get(TSX);
      auto received_operation = require_operation<
        TestMarketDataClient::LoadTickerInfoFromPrefixOperation>(
          operations->pop());
      REQUIRE(received_operation->m_prefix == prefix);
      received_operation->m_result.set({tsx_ticker_info});
    });
    auto au_handler = std::thread([&] {
      auto operations = fixture.m_operations.get(AU);
      auto received_operation = require_operation<
        TestMarketDataClient::LoadTickerInfoFromPrefixOperation>(
          operations->pop());
      REQUIRE(received_operation->m_prefix == prefix);
      received_operation->m_result.set({au_ticker_info});
    });
    auto received_infos = fixture.m_client.load_ticker_info_from_prefix(prefix);
    tsx_handler.join();
    au_handler.join();
    REQUIRE(received_infos.size() == 2);
    std::sort(received_infos.begin(), received_infos.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_ticker < rhs.m_ticker;
      });
    REQUIRE(received_infos[0] == tsx_ticker_info);
    REQUIRE(received_infos[1] == au_ticker_info);
  }
}
