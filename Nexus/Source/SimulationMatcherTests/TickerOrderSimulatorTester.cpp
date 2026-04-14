#include <doctest/doctest.h>
#include <quickfix/FixFields.h>
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

  TEST_CASE("pegged_bid") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::BID, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY), {});
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
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.01"));
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

    SUBCASE("fill_when_ask_drops_to_bid") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("9.99"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.99"));
    }

    SUBCASE("pegged_price_tracks_bid") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.50"), parse_money("9.60"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.50"), parse_money("9.50"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.50"));
    }

    SUBCASE("cancel") {
      simulator.cancel(order);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::PENDING_CANCEL);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::CANCELED);
    }
  }

  TEST_CASE("pegged_ask") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("9.99"), parse_money("10.00"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::ASK, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY), {});
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
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.99"), parse_money("10.00"));
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

    SUBCASE("fill_when_bid_rises_to_ask") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.01"), parse_money("10.00"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.01"));
    }

    SUBCASE("pegged_price_tracks_ask") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.40"), parse_money("10.50"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.50"), parse_money("10.50"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.50"));
    }
  }

  TEST_CASE("pegged_bid_with_limit") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::BID, {}, 100,
      parse_money("10.05"), TimeInForce(TimeInForce::Type::DAY), {});
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
        ABX, parse_money("10.08"), parse_money("10.09"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.05"), parse_money("10.05"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.05"));
    }

    SUBCASE("bid_below_limit_uses_bid") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("10.00"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.00"));
    }
  }

  TEST_CASE("pegged_ask_with_limit") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("9.99"), parse_money("10.00"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::ASK, {}, 100,
      parse_money("9.95"), TimeInForce(TimeInForce::Type::DAY), {});
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
      fixture.m_environment.advance(minutes(1));
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

  TEST_CASE("pegged_bid_with_peg_difference") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::BID, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY),
      {Tag(FIX::FIELD::PegDifference, parse_money("0.03"))});
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
        ABX, parse_money("10.00"), parse_money("9.98"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_effective_price") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.00"), parse_money("9.97"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.97"));
    }

    SUBCASE("tracks_bid_with_offset") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.80"), parse_money("9.90"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.80"), parse_money("9.77"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.77"));
    }
  }

  TEST_CASE("pegged_ask_with_peg_difference") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("9.99"), parse_money("10.00"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())));
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::ASK, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY),
      {Tag(FIX::FIELD::PegDifference, parse_money("0.03"))});
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

    SUBCASE("no_fill_below_effective_price") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.96"), parse_money("10.00"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_effective_price") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("9.97"), parse_money("10.00"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.97"));
    }

    SUBCASE("tracks_ask_with_offset") {
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.10"), parse_money("10.20"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      fixture.m_environment.update_bbo_price(
        ABX, parse_money("10.17"), parse_money("10.20"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.17"));
    }
  }
}
