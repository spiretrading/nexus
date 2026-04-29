#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
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
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields =
      make_limit_order_fields(ABX, Side::BID, 100, parse_money("0.99"));
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
        ABX, parse_money("0.98"), parse_money("0.99"));
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

  TEST_CASE("pegged") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields =
      make_pegged_order_fields(ABX, Side::BID, 100, Money::ZERO, Money::ZERO);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_with_spread") {
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_on_crossed_market") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.00"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.00"));
      REQUIRE(report.m_last_quantity == 100);
    }

    SUBCASE("fill_when_ask_drops_below_bid") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("9.99"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.99"));
    }

    SUBCASE("effective_holds_when_bid_drops") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.50"), parse_money("9.60"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.60"));
    }

    SUBCASE("follows_bid_up") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.50"), parse_money("10.60"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.50"), parse_money("10.50"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.50"));
    }

    SUBCASE("cancel") {
      simulator.cancel(order);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::PENDING_CANCEL);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::CANCELED);
    }
  }

  TEST_CASE("pegged_with_limit") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("9.99"), parse_money("10.00"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::ASK, 100, parse_money("9.95"), Money::ZERO);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("limit_prevents_fill") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.78"), parse_money("9.80"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.95"), parse_money("9.95"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.95"));
    }

    SUBCASE("ask_above_limit_uses_ask") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.00"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.00"));
    }
  }

  TEST_CASE("pegged_with_peg_difference") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::BID, 100, Money::ZERO, parse_money("0.03"));
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_above_effective_price") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.97"), parse_money("9.98"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_effective_price") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.96"), parse_money("9.97"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.97"));
    }

    SUBCASE("follows_bid_up_with_offset") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.20"), parse_money("10.30"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.16"), parse_money("10.17"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.17"));
    }

    SUBCASE("effective_holds_when_bid_drops_with_offset") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.80"), parse_money("9.90"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.90"));
    }
  }

  TEST_CASE("market_peg") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("9.99"), parse_money("10.00"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::ASK, 100, Money::ZERO, Money::ZERO, PegType::MARKET);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);

    SUBCASE("fills_immediately") {
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.99"));
    }
  }

  TEST_CASE("market_peg_with_peg_difference") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.05"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::BID, 100, Money::ZERO, parse_money("0.03"),
      PegType::MARKET);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_with_offset") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.03"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_when_ask_drops") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.02"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.02"));
    }
  }

  TEST_CASE("midpoint_peg") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("9.90"), parse_money("10.10"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::ASK, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY),
      {make_exec_inst(MID_PRICE_PEG)});
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_below_midpoint") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.99"), parse_money("10.10"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_midpoint") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.10"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.00"));
    }

    SUBCASE("follows_midpoint_down") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.70"), parse_money("9.90"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.80"), parse_money("9.90"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.80"));
    }

    SUBCASE("effective_holds_when_midpoint_rises") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.10"), parse_money("10.30"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.10"));
    }
  }

  TEST_CASE("midpoint_peg_with_peg_difference") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.10"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::BID, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY),
      {make_exec_inst(MID_PRICE_PEG),
       make_peg_difference(parse_money("0.02"))});
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_above_effective") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.10"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_effective") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.02"), parse_money("10.03"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.03"));
    }
  }
}
