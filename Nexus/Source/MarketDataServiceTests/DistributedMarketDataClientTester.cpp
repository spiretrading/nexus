#include <future>
#include <thread>
#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/MarketDataService/DistributedMarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  template<typename T>
  auto require_operation(
      std::shared_ptr<TestMarketDataClient::Operation> operation) {
    auto unwrapped_operation = std::get_if<T>(operation.get());
    REQUIRE(unwrapped_operation);
    return std::shared_ptr<T>(operation, unwrapped_operation);
  }

  auto make_operations_queues() {
    auto operations = ScopeTable<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>>(nullptr);
    operations.set(TSX, std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>());
    operations.set(AU, std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>());
    return operations;
  }

  auto make_market_data_clients(const ScopeTable<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>>& operations) {
    auto clients = ScopeTable<std::shared_ptr<MarketDataClient>>(nullptr);
    clients.set(TSX, std::make_shared<MarketDataClient>(
      std::in_place_type<TestMarketDataClient>, operations.get(TSX)));
    clients.set(AU, std::make_shared<MarketDataClient>(
      std::in_place_type<TestMarketDataClient>, operations.get(AU)));
    return clients;
  }

  struct Fixture {
    ScopeTable<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>> m_operations;
    DistributedMarketDataClient m_client;

    Fixture()
      : m_operations(make_operations_queues()),
        m_client(make_market_data_clients(m_operations)) {}
  };
}

TEST_SUITE("DistributedMarketDataClient") {
  TEST_CASE("query_sequenced_order_imbalances") {
    auto fixture = Fixture();
    auto imbalances = std::make_shared<Queue<SequencedOrderImbalance>>();
    auto query = VenueMarketDataQuery();

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
    auto query = VenueMarketDataQuery();

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
    auto query = TickerMarketDataQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBboQuoteOperation>(
          operations->pop());
      auto test_bbo = SequencedValue(
        BboQuote(make_bid(10 * Money::ONE, 100), make_ask(11 * Money::ONE, 100),
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
    auto query = TickerMarketDataQuery();

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
    auto query = TickerMarketDataQuery();

    SUBCASE("exact") {
      query.set_index(parse_ticker("ABC.TSX"));
      query.set_range(Range::REAL_TIME);
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBookQuoteOperation>(
          operations->pop());
      auto test_book_quote = SequencedValue(BookQuote(
        "MMID12", true, TSX, make_bid(10 * Money::ONE, 100),
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
    auto query = TickerMarketDataQuery();

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
      auto test_book_quote = SequencedValue(BookQuote(
        "MMID5", false, ASX, make_ask(20 * Money::ONE, 200),
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

  TEST_CASE("query_sequenced_time_and_sales") {
    auto fixture = Fixture();
    auto time_and_sales = std::make_shared<Queue<SequencedTimeAndSale>>();
    auto query = TickerMarketDataQuery();

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
    auto query = TickerMarketDataQuery();

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
    auto query = TickerInfoQuery();
    query.set_index(parse_ticker("ABC.TSX"));
    auto operations = fixture.m_operations.get(TSX);
    auto result = std::async(std::launch::async, [&] {
      return fixture.m_client.query(query);
    });
    auto received_query = require_operation<
      TestMarketDataClient::TickerInfoQueryOperation>(operations->pop());
    REQUIRE(received_query->m_query.get_index() == parse_ticker("ABC.TSX"));
    auto test_ticker_info = TickerInfo();
    test_ticker_info.m_ticker = parse_ticker("ABC.TSX");
    test_ticker_info.m_name = "Alphabet Inc.";
    test_ticker_info.m_board_lot = 100;
    received_query->m_result.set({test_ticker_info});
    auto received_ticker_info = result.get();
    REQUIRE(received_ticker_info.size() == 1);
    REQUIRE(received_ticker_info.front() == test_ticker_info);
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
      auto test_snapshot = TickerSnapshot(ticker);
      test_snapshot.m_bbo_quote = SequencedValue(BboQuote(
        make_bid(10 * Money::ONE, 100), make_ask(11 * Money::ONE, 100),
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
      auto test_snapshot = TickerSnapshot(ticker);
      test_snapshot.m_bbo_quote = SequencedValue(BboQuote(
        make_bid(20 * Money::ONE, 200), make_ask(21 * Money::ONE, 200),
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

  TEST_CASE("load_session_candlestick") {
    auto fixture = Fixture();

    SUBCASE("exact") {
      auto ticker = parse_ticker("ABC.TSX");
      auto operations = fixture.m_operations.get(TSX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_session_candlestick(ticker);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSessionCandlestickOperation>(
          operations->pop());
      REQUIRE(received_operation->m_ticker == ticker);
      auto test_candlestick = PriceCandlestick();
      received_operation->m_result.set(test_candlestick);
      auto received_candlestick = result.get();
      REQUIRE(received_candlestick == test_candlestick);
    }

    SUBCASE("parent") {
      auto ticker = parse_ticker("S32.ASX");
      auto operations = fixture.m_operations.get(ASX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_session_candlestick(ticker);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSessionCandlestickOperation>(
          operations->pop());
      REQUIRE(received_operation->m_ticker == ticker);
      auto test_candlestick = PriceCandlestick();
      received_operation->m_result.set(test_candlestick);
      auto received_candlestick = result.get();
      REQUIRE(received_candlestick == test_candlestick);
    }

    SUBCASE("unavailable") {
      auto ticker = parse_ticker("BHP.TSXV");
      auto candlestick = fixture.m_client.load_session_candlestick(ticker);
      REQUIRE(candlestick == PriceCandlestick());
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
    auto received_infos =
      fixture.m_client.load_ticker_info_from_prefix(prefix);
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
