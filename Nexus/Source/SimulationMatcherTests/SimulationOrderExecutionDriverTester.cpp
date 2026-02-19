#include <doctest/doctest.h>
#include "Nexus/SimulationMatcher/SimulationOrderExecutionDriver.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto SHOP = parse_ticker("SHOP.TSX");
  const auto TD = parse_ticker("TD.TSX");

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

TEST_SUITE("SimulationOrderExecutionDriver") {
  TEST_CASE("submit") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(SHOP, Money::ONE, 2 * Money::ONE);
    fixture.m_environment.update_bbo_price(
      TD, 10 * Money::ONE, 20 * Money::ONE);
    auto driver = SimulationOrderExecutionDriver(fixture.m_market_data_client,
      std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info1 = OrderInfo();
    info1.m_fields = make_limit_order_fields(SHOP, Side::BID, 100, Money::ONE);
    info1.m_id = 1;
    info1.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto info2 = OrderInfo();
    info2.m_fields =
      make_limit_order_fields(TD, Side::ASK, 200, 30 * Money::ONE);
    info2.m_id = 2;
    info2.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order1 = driver.submit(info1);
    auto order2 = driver.submit(info2);
    REQUIRE(order1);
    REQUIRE(order2);
    REQUIRE(order1->get_info().m_fields.m_ticker == SHOP);
    REQUIRE(order2->get_info().m_fields.m_ticker == TD);
    auto reports1 = std::make_shared<Queue<ExecutionReport>>();
    auto reports2 = std::make_shared<Queue<ExecutionReport>>();
    order1->get_publisher().monitor(reports1);
    order2->get_publisher().monitor(reports2);
    auto report1 = reports1->pop();
    REQUIRE(report1.m_status == OrderStatus::PENDING_NEW);
    report1 = reports1->pop();
    REQUIRE(report1.m_status == OrderStatus::NEW);
    auto report2 = reports2->pop();
    REQUIRE(report2.m_status == OrderStatus::PENDING_NEW);
    report2 = reports2->pop();
    REQUIRE(report2.m_status == OrderStatus::NEW);
  }
}
