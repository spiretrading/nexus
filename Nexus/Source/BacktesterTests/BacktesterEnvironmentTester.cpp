#include <doctest/doctest.h>
#include "Nexus/Backtester/BacktesterClients.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Venues;

TEST_SUITE("BacktesterEnvironment") {
  TEST_CASE("fill_limit_order") {
    auto start_time = time_from_string("2020-12-11 00:00:10");
    auto data_store = LocalHistoricalDataStore();
    auto ticker = parse_ticker("TST.TSXV");
    auto timestamp = start_time - seconds(1);
    auto bbo = SequencedValue(IndexedValue(BboQuote(
      make_bid(99 * Money::CENT, 100), make_ask(Money::ONE, 100), timestamp),
      ticker), encode(timestamp, Beam::Sequence(1)));
    data_store.store(bbo);
    timestamp = start_time + seconds(1);
    bbo = SequencedValue(IndexedValue(BboQuote(make_bid(98 * Money::CENT, 100),
      make_ask(99 * Money::CENT, 100), timestamp), ticker),
      encode(timestamp, Beam::Sequence(2)));
    data_store.store(bbo);
    auto test_environment = TestEnvironment(HistoricalDataStore(&data_store));
    auto backtester = BacktesterEnvironment(start_time, start_time + hours(8),
      Clients(std::in_place_type<TestClients>, Ref(test_environment)));
    auto clients = BacktesterClients(Ref(backtester));
    auto& order_execution_client = clients.get_order_execution_client();
    auto order = order_execution_client.submit(
      make_limit_order_fields(ticker, Side::BID, 100, 99 * Money::CENT));
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    REQUIRE(reports->pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports->pop().m_status == OrderStatus::NEW);
    auto fill = reports->pop();
    REQUIRE(fill.m_status == OrderStatus::FILLED);
    REQUIRE(fill.m_last_price == 99 * Money::CENT);
    REQUIRE(fill.m_last_quantity == 100);
  }

  TEST_CASE("snapshot_separates_listings_by_venue") {
    auto start_time = time_from_string("2020-12-11 00:00:10");
    auto data_store = LocalHistoricalDataStore();
    auto ticker = parse_ticker("TST.TSXV");
    auto timestamp = start_time - seconds(1);
    auto bbo = SequencedValue(IndexedValue(BboQuote(
      make_bid(99 * Money::CENT, 100), make_ask(Money::ONE, 100), timestamp),
      ticker), encode(timestamp, Beam::Sequence(1)));
    data_store.store(bbo);
    auto listing = SequencedValue(IndexedValue(BookQuote("A", false, TSXV,
      Quote(99 * Money::CENT, 500, Side::BID), timestamp), ticker),
      encode(timestamp, Beam::Sequence(2)));
    data_store.store(listing);
    listing = SequencedValue(IndexedValue(BookQuote("A", false, XATS,
      Quote(99 * Money::CENT, 500, Side::BID), timestamp), ticker),
      encode(timestamp, Beam::Sequence(3)));
    data_store.store(listing);
    auto sale = SequencedValue(IndexedValue(
      TimeAndSale(start_time + seconds(1), 99 * Money::CENT, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "CDX"), ticker), encode(start_time + seconds(1), Beam::Sequence(4)));
    data_store.store(sale);
    auto test_environment = TestEnvironment(HistoricalDataStore(&data_store));
    auto backtester = BacktesterEnvironment(start_time, start_time + hours(1),
      Clients(std::in_place_type<TestClients>, Ref(test_environment)));
    auto clients = BacktesterClients(Ref(backtester));
    auto& order_execution_client = clients.get_order_execution_client();
    auto order = order_execution_client.submit(make_limit_order_fields(
      ticker, CurrencyId::NONE, Side::BID, "TSX", 100, 99 * Money::CENT));
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    REQUIRE(reports->pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports->pop().m_status == OrderStatus::NEW);
    backtester.wait();
    REQUIRE(!reports->try_pop());
  }

  TEST_CASE("replayed_book_quote_sizes_are_absolute") {
    auto start_time = time_from_string("2020-12-11 00:00:10");
    auto data_store = LocalHistoricalDataStore();
    auto ticker = parse_ticker("TST.TSXV");
    auto timestamp = start_time + seconds(1);
    auto listing = SequencedValue(IndexedValue(BookQuote("A", false, TSXV,
      Quote(99 * Money::CENT, 500, Side::BID), timestamp), ticker),
      encode(timestamp, Beam::Sequence(1)));
    data_store.store(listing);
    timestamp = start_time + seconds(2);
    listing = SequencedValue(IndexedValue(BookQuote("A", false, TSXV,
      Quote(99 * Money::CENT, 200, Side::BID), timestamp), ticker),
      encode(timestamp, Beam::Sequence(2)));
    data_store.store(listing);
    auto test_environment = TestEnvironment(HistoricalDataStore(&data_store));
    auto backtester = BacktesterEnvironment(start_time, start_time + hours(1),
      Clients(std::in_place_type<TestClients>, Ref(test_environment)));
    auto clients = BacktesterClients(Ref(backtester));
    auto& market_data_client = clients.get_market_data_client();
    auto quotes = std::make_shared<Queue<BookQuote>>();
    market_data_client.query(Beam::make_current_query(ticker), quotes);
    backtester.wait();
    auto snapshot = market_data_client.load_snapshot(ticker);
    REQUIRE(snapshot.m_bids.size() == 1);
    REQUIRE(snapshot.m_bids[0]->m_quote.m_size == 200);
  }

  TEST_CASE("wait") {
    auto start_time = time_from_string("2020-12-11 00:00:10");
    auto end_time = start_time + hours(1);
    auto data_store = LocalHistoricalDataStore();
    auto ticker = parse_ticker("TST.TSXV");
    auto timestamp = start_time + seconds(1);
    auto bbo = SequencedValue(IndexedValue(BboQuote(
      make_bid(99 * Money::CENT, 100), make_ask(Money::ONE, 100), timestamp),
      ticker), encode(timestamp, Beam::Sequence(1)));
    data_store.store(bbo);
    auto test_environment = TestEnvironment(HistoricalDataStore(&data_store));
    auto backtester = BacktesterEnvironment(start_time, end_time,
      Clients(std::in_place_type<TestClients>, Ref(test_environment)));
    auto clients = BacktesterClients(Ref(backtester));
    auto& order_execution_client = clients.get_order_execution_client();
    order_execution_client.submit(
      make_limit_order_fields(ticker, Side::BID, 100, 99 * Money::CENT));
    backtester.wait();
    REQUIRE(backtester.get_event_handler().get_time() >= end_time);
  }

  TEST_CASE("make_portfolio") {
    auto start_time = time_from_string("2020-12-11 00:00:10");
    auto data_store = LocalHistoricalDataStore();
    auto ticker = parse_ticker("TST.TSXV");
    auto timestamp = start_time - seconds(1);
    auto bbo = SequencedValue(IndexedValue(BboQuote(
      make_bid(99 * Money::CENT, 100), make_ask(Money::ONE, 100), timestamp),
      ticker), encode(timestamp, Beam::Sequence(1)));
    data_store.store(bbo);
    timestamp = start_time + seconds(1);
    bbo = SequencedValue(IndexedValue(BboQuote(make_bid(98 * Money::CENT, 100),
      make_ask(99 * Money::CENT, 100), timestamp), ticker),
      encode(timestamp, Beam::Sequence(2)));
    data_store.store(bbo);
    auto test_environment = TestEnvironment(HistoricalDataStore(&data_store));
    auto backtester = BacktesterEnvironment(start_time, start_time + hours(8),
      Clients(std::in_place_type<TestClients>, Ref(test_environment)));
    auto clients = BacktesterClients(Ref(backtester));
    clients.get_order_execution_client().submit(
      make_limit_order_fields(ticker, Side::BID, 100, 99 * Money::CENT));
    backtester.wait();
    auto portfolio = make_portfolio(clients);
    auto& inventory = portfolio.get_bookkeeper().get_inventory(ticker);
    REQUIRE(inventory.m_position.m_quantity == 100);
    auto entry = portfolio.get_entries().find(ticker);
    REQUIRE(entry != portfolio.get_entries().end());
    REQUIRE(entry->second.m_unrealized == -Money::ONE);
  }
}
