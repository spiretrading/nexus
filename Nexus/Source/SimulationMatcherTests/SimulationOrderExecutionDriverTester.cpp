#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/SimulationMatcher/SimulationOrderExecutionDriver.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto SHOP = parse_ticker("SHOP.TSX");

  struct Fixture {
    TestEnvironment m_environment;
    MarketDataClient m_market_data_client;

    Fixture()
      : m_environment(time_from_string("2025-08-14 09:00:00.000")),
        m_market_data_client(
          make_market_data_client(m_environment, "simulator")) {}

    SimulationOrderExecutionDriver make_driver() {
      return SimulationOrderExecutionDriver(
        m_market_data_client, std::make_unique<TestTimeClient>(
          Ref(m_environment.get_time_environment())));
    }

    OrderInfo make_bid_limit(Money price) {
      auto info = OrderInfo();
      info.m_fields = make_limit_order_fields(SHOP, Side::BID, 100, price);
      info.m_id = 1;
      info.m_timestamp = m_environment.get_time_environment().get_time();
      return info;
    }
  };
}

TEST_SUITE("SimulationOrderExecutionDriver") {
  TEST_CASE("submit_publishes_without_explicit_flush") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(SHOP, Money::ONE, 2 * Money::ONE);
    auto driver = fixture.make_driver();
    auto order = driver.submit(fixture.make_bid_limit(Money::ONE));
    REQUIRE(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    REQUIRE(reports->pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports->pop().m_status == OrderStatus::NEW);
  }

  TEST_CASE("market_data_triggers_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(SHOP, Money::ONE, 2 * Money::ONE);
    auto driver = fixture.make_driver();
    auto order = driver.submit(fixture.make_bid_limit(Money::ONE));
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    REQUIRE(reports->pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports->pop().m_status == OrderStatus::NEW);
    fixture.m_environment.update_bbo_price(SHOP, Money::ONE, Money::ONE);
    REQUIRE(reports->pop().m_status == OrderStatus::FILLED);
  }
}
