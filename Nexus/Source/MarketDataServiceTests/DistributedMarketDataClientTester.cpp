#include <future>
#include <thread>
#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataService/DistributedMarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

namespace {
  template<typename T>
  auto require_operation(
      std::shared_ptr<TestMarketDataClient::Operation> operation) {
    auto unwrapped_operation = std::get_if<T>(operation.get());
    REQUIRE(unwrapped_operation);
    return std::shared_ptr<T>(operation, unwrapped_operation);
  }

  auto make_operations_queues() {
    auto operations = RegionMap<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>>(nullptr);
    operations.set(TSX, std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>());
    operations.set(US, std::make_shared<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>());
    return operations;
  }

  auto make_market_data_clients(const RegionMap<std::shared_ptr<
      Queue<std::shared_ptr<TestMarketDataClient::Operation>>>>& operations) {
    auto clients = RegionMap<std::shared_ptr<MarketDataClient>>(nullptr);
    clients.set(TSX, std::make_shared<MarketDataClient>(
      std::in_place_type<TestMarketDataClient>, operations.get(TSX)));
    clients.set(US, std::make_shared<MarketDataClient>(
      std::in_place_type<TestMarketDataClient>, operations.get(US)));
    return clients;
  }

  struct Fixture {
    RegionMap<std::shared_ptr<
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
      query.SetIndex(TSX);
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
          operations->Pop());
      auto test_imbalance = SequencedValue(
        OrderImbalance(Security("ABC", TSX), Side::BID, 100, Money::ONE,
          time_from_string("2024-06-12 13:05:12:00")), Queries::Sequence(100));
      received_query->m_queue.Push(test_imbalance);
      auto received_imbalance = imbalances->Pop();
      REQUIRE(received_imbalance == test_imbalance);
    }
    SUBCASE("parent") {
      query.SetIndex(NYSE);
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedOrderImbalanceOperation>(
          operations->Pop());
      auto test_imbalance = SequencedValue(
        OrderImbalance(Security("IBM", NYSE), Side::ASK, 200, 3 * Money::ONE,
          time_from_string("2025-02-18 17:23:30:12")), Queries::Sequence(200));
      received_query->m_queue.Push(test_imbalance);
      auto received_imbalance = imbalances->Pop();
      REQUIRE(received_imbalance == test_imbalance);
    }
    SUBCASE("unavailable") {
      query.SetIndex(ASX);
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, imbalances);
      REQUIRE_THROWS_AS(imbalances->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_order_imbalances") {
    auto fixture = Fixture();
    auto imbalances = std::make_shared<Queue<OrderImbalance>>();
    auto query = VenueMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(TSX);
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryOrderImbalanceOperation>(operations->Pop());
      auto test_imbalance = OrderImbalance(Security("ABC", TSX), Side::BID,
        100, Money::ONE, time_from_string("2024-06-12 13:05:12:00"));
      received_query->m_queue.Push(test_imbalance);
      auto received_imbalance = imbalances->Pop();
      REQUIRE(received_imbalance == test_imbalance);
    }
    SUBCASE("parent") {
      query.SetIndex(NYSE);
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, imbalances);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QueryOrderImbalanceOperation>(operations->Pop());
      auto test_imbalance = OrderImbalance(Security("IBM", NYSE), Side::ASK,
        200, 3 * Money::ONE, time_from_string("2025-02-18 17:23:30:12"));
      received_query->m_queue.Push(test_imbalance);
      auto received_imbalance = imbalances->Pop();
      REQUIRE(received_imbalance == test_imbalance);
    }
    SUBCASE("unavailable") {
      query.SetIndex(ASX);
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, imbalances);
      REQUIRE_THROWS_AS(imbalances->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_bbo_quotes") {
    auto fixture = Fixture();
    auto bbo_quotes = std::make_shared<Queue<SequencedBboQuote>>();
    auto query = SecurityMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(Security("ABC", TSX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBboQuoteOperation>(
          operations->Pop());
      auto test_bbo = SequencedValue(BboQuote(
        Quote(10 * Money::ONE, 100, Side::BID),
        Quote(11 * Money::ONE, 100, Side::ASK),
        time_from_string("2024-06-12 13:05:12:00")), Queries::Sequence(100));
      received_query->m_queue.Push(test_bbo);
      auto received_bbo = bbo_quotes->Pop();
      REQUIRE(received_bbo == test_bbo);
    }
    SUBCASE("parent") {
      query.SetIndex(Security("IBM", NYSE));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBboQuoteOperation>(
          operations->Pop());
      auto test_bbo = SequencedValue(BboQuote(
        Quote(20 * Money::ONE, 200, Side::BID),
        Quote(21 * Money::ONE, 200, Side::ASK),
        time_from_string("2025-02-18 17:23:30:12")), Queries::Sequence(200));
      received_query->m_queue.Push(test_bbo);
      auto received_bbo = bbo_quotes->Pop();
      REQUIRE(received_bbo == test_bbo);
    }
    SUBCASE("unavailable") {
      query.SetIndex(Security("BHP", ASX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, bbo_quotes);
      REQUIRE_THROWS_AS(bbo_quotes->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_bbo_quotes") {
    auto fixture = Fixture();
    auto bbo_quotes = std::make_shared<Queue<BboQuote>>();
    auto query = SecurityMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(Security("ABC", TSX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBboQuoteOperation>(operations->Pop());
      auto test_bbo = BboQuote(
        Quote(10 * Money::ONE, 100, Side::BID),
        Quote(11 * Money::ONE, 100, Side::ASK),
        time_from_string("2024-06-12 13:05:12:00"));
      received_query->m_queue.Push(test_bbo);
      auto received_bbo = bbo_quotes->Pop();
      REQUIRE(received_bbo == test_bbo);
    }
    SUBCASE("parent") {
      query.SetIndex(Security("IBM", NYSE));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, bbo_quotes);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBboQuoteOperation>(operations->Pop());
      auto test_bbo = BboQuote(
        Quote(20 * Money::ONE, 200, Side::BID),
        Quote(21 * Money::ONE, 200, Side::ASK),
        time_from_string("2025-02-18 17:23:30:12"));
      received_query->m_queue.Push(test_bbo);
      auto received_bbo = bbo_quotes->Pop();
      REQUIRE(received_bbo == test_bbo);
    }
    SUBCASE("unavailable") {
      query.SetIndex(Security("BHP", ASX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, bbo_quotes);
      REQUIRE_THROWS_AS(bbo_quotes->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_book_quotes") {
    auto fixture = Fixture();
    auto book_quotes = std::make_shared<Queue<SequencedBookQuote>>();
    auto query = SecurityMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(Security("ABC", TSX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBookQuoteOperation>(
          operations->Pop());
      auto test_book_quote = SequencedValue(BookQuote(
        "MMID12", true, TSX, Quote(10 * Money::ONE, 100, Side::BID),
        time_from_string("2024-06-12 13:05:12:00")), Queries::Sequence(100));
      received_query->m_queue.Push(test_book_quote);
      auto received_book_quote = book_quotes->Pop();
      REQUIRE(received_book_quote == test_book_quote);
    }
    SUBCASE("parent") {
      query.SetIndex(Security("IBM", NYSE));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedBookQuoteOperation>(
          operations->Pop());
      auto test_book_quote = SequencedValue(BookQuote(
        "MMID5", false, NYSE, Quote(20 * Money::ONE, 200, Side::ASK),
        time_from_string("2025-02-18 17:23:30:12")), Queries::Sequence(200));
      received_query->m_queue.Push(test_book_quote);
      auto received_book_quote = book_quotes->Pop();
      REQUIRE(received_book_quote == test_book_quote);
    }
    SUBCASE("unavailable") {
      query.SetIndex(Security("BHP", ASX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, book_quotes);
      REQUIRE_THROWS_AS(book_quotes->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_book_quotes") {
    auto fixture = Fixture();
    auto book_quotes = std::make_shared<Queue<BookQuote>>();
    auto query = SecurityMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(Security("ABC", TSX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBookQuoteOperation>(operations->Pop());
      auto test_book_quote = SequencedValue(BookQuote(
        "MMID12", true, TSX, Quote(10 * Money::ONE, 100, Side::BID),
        time_from_string("2024-06-12 13:05:12:00")), Queries::Sequence(100));
      received_query->m_queue.Push(test_book_quote);
      auto received_book_quote = book_quotes->Pop();
      REQUIRE(received_book_quote == test_book_quote);
    }
    SUBCASE("parent") {
      query.SetIndex(Security("IBM", NYSE));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, book_quotes);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QueryBookQuoteOperation>(operations->Pop());
      auto test_book_quote = SequencedValue(BookQuote(
        "MMID5", false, NYSE, Quote(20 * Money::ONE, 200, Side::ASK),
        time_from_string("2025-02-18 17:23:30:12")), Queries::Sequence(200));
      received_query->m_queue.Push(test_book_quote);
      auto received_book_quote = book_quotes->Pop();
      REQUIRE(received_book_quote == test_book_quote);
    }
    SUBCASE("unavailable") {
      query.SetIndex(Security("BHP", ASX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, book_quotes);
      REQUIRE_THROWS_AS(book_quotes->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_sequenced_time_and_sales") {
    auto fixture = Fixture();
    auto time_and_sales = std::make_shared<Queue<SequencedTimeAndSale>>();
    auto query = SecurityMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(Security("ABC", TSX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedTimeAndSaleOperation>(
          operations->Pop());
      auto test_time_and_sale = SequencedValue(TimeAndSale(
        time_from_string("2024-07-09 12:00:00.123"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "TSX", "B1", "S1"), Queries::Sequence(100));
      received_query->m_queue.Push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->Pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }
    SUBCASE("parent") {
      query.SetIndex(Security("IBM", NYSE));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QuerySequencedTimeAndSaleOperation>(
          operations->Pop());
      auto test_time_and_sale = SequencedValue(TimeAndSale(
        time_from_string("2025-02-18 17:23:30.12"), 150 * Money::ONE, 200,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "NYSE", "B52", "S46"), Queries::Sequence(200));
      received_query->m_queue.Push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->Pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }
    SUBCASE("unavailable") {
      query.SetIndex(Security("BHP", ASX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, time_and_sales);
      REQUIRE_THROWS_AS(time_and_sales->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_time_and_sales") {
    auto fixture = Fixture();
    auto time_and_sales = std::make_shared<Queue<TimeAndSale>>();
    auto query = SecurityMarketDataQuery();
    SUBCASE("exact") {
      query.SetIndex(Security("ABC", TSX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(TSX);
      auto received_query = require_operation<
        TestMarketDataClient::QueryTimeAndSaleOperation>(operations->Pop());
      auto test_time_and_sale = TimeAndSale(
        time_from_string("2024-07-09 12:00:00.123"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "TSX", "B11", "S76");
      received_query->m_queue.Push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->Pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }
    SUBCASE("parent") {
      query.SetIndex(Security("IBM", NYSE));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, time_and_sales);
      auto operations = fixture.m_operations.get(NYSE);
      auto received_query = require_operation<
        TestMarketDataClient::QueryTimeAndSaleOperation>(operations->Pop());
      auto test_time_and_sale = TimeAndSale(
        time_from_string("2025-02-18 17:23:30.12"), 150 * Money::ONE, 200,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "NYSE", "B4", "S99");
      received_query->m_queue.Push(test_time_and_sale);
      auto received_time_and_sale = time_and_sales->Pop();
      REQUIRE(received_time_and_sale == test_time_and_sale);
    }
    SUBCASE("unavailable") {
      query.SetIndex(Security("BHP", ASX));
      query.SetRange(Range::RealTime());
      fixture.m_client.query(query, time_and_sales);
      REQUIRE_THROWS_AS(time_and_sales->Pop(), PipeBrokenException);
    }
  }

  TEST_CASE("query_security_info") {
    auto fixture = Fixture();
    auto query = SecurityInfoQuery();
    query.SetIndex(Security("ABC", TSX));
    auto operations = fixture.m_operations.get(TSX);
    auto result = std::async(std::launch::async, [&] {
      return fixture.m_client.query(query);
    });
    auto received_query = require_operation<
      TestMarketDataClient::SecurityInfoQueryOperation>(operations->Pop());
    REQUIRE(received_query->m_query.GetIndex() == Security("ABC", TSX));
    auto test_security_info = SecurityInfo();
    test_security_info.m_security = Security("ABC", TSX);
    test_security_info.m_name = "Alphabet Inc.";
    test_security_info.m_sector = "Technology";
    test_security_info.m_board_lot = 100;
    received_query->m_result.set({test_security_info});
    auto received_security_info = result.get();
    REQUIRE(received_security_info.size() == 1);
    REQUIRE(received_security_info.front() == test_security_info);
  }

  TEST_CASE("load_snapshot") {
    auto fixture = Fixture();
    SUBCASE("exact") {
      auto security = Security("ABC", TSX);
      auto operations = fixture.m_operations.get(TSX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_snapshot(security);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSecuritySnapshotOperation>(operations->Pop());
      REQUIRE(received_operation->m_security == security);
      auto test_snapshot = SecuritySnapshot();
      test_snapshot.m_bbo_quote = SequencedValue(BboQuote(
        Quote(10 * Money::ONE, 100, Side::BID),
        Quote(11 * Money::ONE, 100, Side::ASK),
        time_from_string("2024-06-12 13:05:12:00")), Queries::Sequence(100));
      received_operation->m_result.set(test_snapshot);
      auto received_snapshot = result.get();
      REQUIRE(received_snapshot.m_bbo_quote == test_snapshot.m_bbo_quote);
    }
    SUBCASE("parent") {
      auto security = Security("IBM", NYSE);
      auto operations = fixture.m_operations.get(NYSE);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_snapshot(security);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSecuritySnapshotOperation>(operations->Pop());
      REQUIRE(received_operation->m_security == security);
      auto test_snapshot = SecuritySnapshot();
      test_snapshot.m_bbo_quote = SequencedValue(BboQuote(
        Quote(20 * Money::ONE, 200, Side::BID),
        Quote(21 * Money::ONE, 200, Side::ASK),
        time_from_string("2025-02-18 17:23:30:12")), Queries::Sequence(200));
      received_operation->m_result.set(test_snapshot);
      auto received_snapshot = result.get();
      REQUIRE(received_snapshot.m_bbo_quote == test_snapshot.m_bbo_quote);
    }
    SUBCASE("unavailable") {
      auto security = Security("BHP", ASX);
      auto snapshot = fixture.m_client.load_snapshot(security);
      REQUIRE(snapshot.m_bbo_quote == SequencedBboQuote());
      REQUIRE(snapshot.m_asks.empty());
      REQUIRE(snapshot.m_bids.empty());
    }
  }

  TEST_CASE("load_technicals") {
    auto fixture = Fixture();
    SUBCASE("exact") {
      auto security = Security("ABC", TSX);
      auto operations = fixture.m_operations.get(TSX);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_technicals(security);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSecurityTechnicalsOperation>(
          operations->Pop());
      REQUIRE(received_operation->m_security == security);
      auto test_technicals = SecurityTechnicals();
      test_technicals.m_close = Money::ONE;
      test_technicals.m_high = 2 * Money::ONE;
      test_technicals.m_low = Money::CENT;
      test_technicals.m_open = Money::ONE + Money::CENT;
      test_technicals.m_volume = 100;
      received_operation->m_result.set(test_technicals);
      auto received_technicals = result.get();
      TestJsonEquality(received_technicals, test_technicals);
    }
    SUBCASE("parent") {
      auto security = Security("IBM", NYSE);
      auto operations = fixture.m_operations.get(NYSE);
      auto result = std::async(std::launch::async, [&] {
        return fixture.m_client.load_technicals(security);
      });
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSecurityTechnicalsOperation>(
          operations->Pop());
      REQUIRE(received_operation->m_security == security);
      auto test_technicals = SecurityTechnicals();
      test_technicals.m_close = 150 * Money::ONE;
      test_technicals.m_high = 152 * Money::ONE;
      test_technicals.m_low = 148 * Money::ONE;
      test_technicals.m_open = 151 * Money::ONE;
      test_technicals.m_volume = 10000;
      received_operation->m_result.set(test_technicals);
      auto received_technicals = result.get();
      TestJsonEquality(received_technicals, test_technicals);
    }
    SUBCASE("unavailable") {
      auto security = Security("BHP", ASX);
      auto technicals = fixture.m_client.load_technicals(security);
      TestJsonEquality(technicals, SecurityTechnicals());
    }
  }

  TEST_CASE("load_security_info_from_prefix") {
    auto fixture = Fixture();
    auto prefix = "A";
    auto tsx_security_info = SecurityInfo();
    tsx_security_info.m_security = Security("ABC", TSX);
    tsx_security_info.m_name = "Alphabet Inc. Class C";
    auto us_security_info = SecurityInfo();
    us_security_info.m_security = Security("AAPL", NYSE);
    us_security_info.m_name = "Apple Inc.";
    auto tsx_handler = std::thread([&] {
      auto operations = fixture.m_operations.get(TSX);
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSecurityInfoFromPrefixOperation>(
          operations->Pop());
      REQUIRE(received_operation->m_prefix == prefix);
      received_operation->m_result.set({tsx_security_info});
    });
    auto us_handler = std::thread([&] {
      auto operations = fixture.m_operations.get(US);
      auto received_operation = require_operation<
        TestMarketDataClient::LoadSecurityInfoFromPrefixOperation>(
          operations->Pop());
      REQUIRE(received_operation->m_prefix == prefix);
      received_operation->m_result.set({us_security_info});
    });
    auto received_infos =
      fixture.m_client.load_security_info_from_prefix(prefix);
    tsx_handler.join();
    us_handler.join();
    REQUIRE(received_infos.size() == 2);
    std::sort(received_infos.begin(), received_infos.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_security < rhs.m_security;
      });
    REQUIRE(received_infos[0] == us_security_info);
    REQUIRE(received_infos[1] == tsx_security_info);
  }
}
