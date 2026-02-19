#include <doctest/doctest.h>
#include "Nexus/SimulationMatcher/TickerOrderSimulator.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto ABX = parse_ticker("ABX.TSX");

  struct Fixture {
    TestEnvironment m_environment;
    MarketDataClient m_market_data_client;

    Fixture()
      : m_environment(time_from_string("2025-08-14 09:00:00.000")),
        m_market_data_client(
          make_market_data_client(m_environment, "simulator")) {
    }
  };
}

TEST_SUITE("TickerOrderSimulator") {
  TEST_CASE("submit") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(ABX, Money::ONE, Money::ONE);
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields =
      make_limit_order_fields(ABX, Side::BID, 100, 99 * Money::CENT);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    REQUIRE(report.m_timestamp ==
      fixture.m_environment.get_time_environment().get_time());
    fixture.m_environment.advance(minutes(1));

    SUBCASE("fill") {
      fixture.m_environment.update_bbo_price(
        ABX, 98 * Money::CENT, 99 * Money::CENT);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_timestamp ==
        fixture.m_environment.get_time_environment().get_time());
    }

    SUBCASE("cancel") {
      simulator.cancel(order);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::PENDING_CANCEL);
      REQUIRE(report.m_timestamp ==
        fixture.m_environment.get_time_environment().get_time());
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::CANCELED);
      REQUIRE(report.m_timestamp ==
        fixture.m_environment.get_time_environment().get_time());
    }
  }
}
