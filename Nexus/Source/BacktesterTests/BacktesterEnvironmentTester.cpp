#include <doctest/doctest.h>
#include "Nexus/Backtester/BacktesterClients.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

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
    auto backtester = BacktesterEnvironment(start_time,
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
}
