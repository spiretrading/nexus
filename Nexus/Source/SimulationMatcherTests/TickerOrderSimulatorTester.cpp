#include <doctest/doctest.h>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
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
    std::shared_ptr<SimulationExecutionReportQueue> m_reports;

    Fixture()
      : m_environment(time_from_string("2025-08-14 09:00:00.000")),
        m_market_data_client(
          make_market_data_client(m_environment, "simulator")),
        m_reports(std::make_shared<SimulationExecutionReportQueue>()) {
    }
  };

  void update_bbo_price(
      Fixture& fixture, auto& simulator, Money bid_price, Money ask_price) {
    fixture.m_environment.update_bbo_price(ABX, bid_price, ask_price);
    simulator.update(
      BboQuote(make_bid(bid_price, 100), make_ask(ask_price, 100),
        fixture.m_environment.get_time_environment().get_time()));
  }

  auto make_simulator(Fixture& fixture) {
    return TickerOrderSimulator(fixture.m_market_data_client, ABX,
      std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())), fixture.m_reports);
  }

  std::shared_ptr<PrimitiveOrder> submit_order(Fixture& fixture,
      auto& simulator, OrderId id, const OrderFields& fields) {
    auto info = OrderInfo();
    info.m_fields = fields;
    info.m_id = id;
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    return order;
  }

  std::shared_ptr<PrimitiveOrder> submit_limit_order(Fixture& fixture,
      auto& simulator, OrderId id, Side side, Quantity quantity, Money price) {
    return submit_order(fixture, simulator, id,
      make_limit_order_fields(ABX, side, quantity, price));
  }

  std::shared_ptr<Queue<ExecutionReport>> monitor_reports(
      const std::shared_ptr<PrimitiveOrder>& order) {
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    return reports;
  }

  std::shared_ptr<PrimitiveOrder> submit_limit_order(Fixture& fixture,
      auto& simulator, OrderId id, Side side, Quantity quantity, Money price,
      Destination destination) {
    return submit_order(fixture, simulator, id, make_limit_order_fields(
      ABX, CurrencyId::NONE, side, std::move(destination), quantity, price));
  }

  void publish_time_and_sale(Fixture& fixture, auto& simulator, Money price,
      Quantity size, const std::string& code = "@",
      const std::string& market_center = "TSE") {
    simulator.update(TimeAndSale(
      fixture.m_environment.get_time_environment().get_time(), price, size,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, code),
      market_center));
  }

  void publish_book_quote(Fixture& fixture, auto& simulator,
      const std::string& mpid, Venue venue, Side side, Money price,
      Quantity size) {
    simulator.update(BookQuote(mpid, false, venue, Quote(price, size, side),
      fixture.m_environment.get_time_environment().get_time()));
  }
}

TEST_SUITE("TickerOrderSimulator") {
  TEST_CASE("submit") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields =
      make_limit_order_fields(ABX, Side::BID, 100, parse_money("0.99"));
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
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
      update_bbo_price(
        fixture, simulator, parse_money("0.98"), parse_money("0.99"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_timestamp ==
        fixture.m_environment.get_time_environment().get_time());
    }

    SUBCASE("cancel") {
      simulator.cancel(order);
      fixture.m_reports->flush();
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
        Ref(fixture.m_environment.get_time_environment())), fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields =
      make_pegged_order_fields(ABX, Side::BID, 100, Money::ZERO, Money::ZERO);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
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
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("10.00"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.00"));
      REQUIRE(report.m_last_quantity == 100);
    }

    SUBCASE("fill_when_ask_drops_below_bid") {
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("9.99"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.99"));
    }

    SUBCASE("effective_holds_when_bid_drops") {
      update_bbo_price(
        fixture, simulator, parse_money("9.50"), parse_money("9.60"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.60"));
    }

    SUBCASE("follows_bid_up") {
      update_bbo_price(
        fixture, simulator, parse_money("10.50"), parse_money("10.60"));
      REQUIRE(!reports->try_pop());
      update_bbo_price(
        fixture, simulator, parse_money("10.50"), parse_money("10.50"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.50"));
    }

    SUBCASE("cancel") {
      simulator.cancel(order);
      fixture.m_reports->flush();
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
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::ASK, 100, parse_money("9.95"), Money::ZERO);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("limit_prevents_fill") {
      update_bbo_price(
        fixture, simulator, parse_money("9.78"), parse_money("9.80"));
      REQUIRE(!reports->try_pop());
      update_bbo_price(
        fixture, simulator, parse_money("9.95"), parse_money("9.95"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.95"));
    }

    SUBCASE("ask_above_limit_uses_ask") {
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("10.00"));
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
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::BID, 100, Money::ZERO, parse_money("0.03"));
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_above_effective_price") {
      update_bbo_price(
        fixture, simulator, parse_money("9.97"), parse_money("9.98"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_effective_price") {
      update_bbo_price(
        fixture, simulator, parse_money("9.96"), parse_money("9.97"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.97"));
    }

    SUBCASE("follows_bid_up_with_offset") {
      update_bbo_price(
        fixture, simulator, parse_money("10.20"), parse_money("10.30"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      update_bbo_price(
        fixture, simulator, parse_money("10.16"), parse_money("10.17"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.17"));
    }

    SUBCASE("effective_holds_when_bid_drops_with_offset") {
      update_bbo_price(
        fixture, simulator, parse_money("9.80"), parse_money("9.90"));
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
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::ASK, 100, Money::ZERO, Money::ZERO, PegType::MARKET);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
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
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = make_pegged_order_fields(
      ABX, Side::BID, 100, Money::ZERO, parse_money("0.03"),
      PegType::MARKET);
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_with_offset") {
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("10.03"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_when_ask_drops") {
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("10.02"));
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
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::ASK, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY),
      {make_exec_inst(MID_PRICE_PEG)});
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_below_midpoint") {
      update_bbo_price(
        fixture, simulator, parse_money("9.99"), parse_money("10.10"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_midpoint") {
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("10.10"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.00"));
    }

    SUBCASE("follows_midpoint_down") {
      update_bbo_price(
        fixture, simulator, parse_money("9.70"), parse_money("9.90"));
      REQUIRE(!reports->try_pop());
      fixture.m_environment.advance(minutes(1));
      update_bbo_price(
        fixture, simulator, parse_money("9.80"), parse_money("9.90"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("9.80"));
    }

    SUBCASE("effective_holds_when_midpoint_rises") {
      update_bbo_price(
        fixture, simulator, parse_money("10.10"), parse_money("10.30"));
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
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = OrderFields(
      {}, ABX, CurrencyId::NONE, OrderType::PEGGED, Side::BID, {}, 100,
      Money::ZERO, TimeInForce(TimeInForce::Type::DAY),
      {make_exec_inst(MID_PRICE_PEG),
       make_peg_difference(parse_money("0.02"))});
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::NEW);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_above_effective") {
      update_bbo_price(
        fixture, simulator, parse_money("10.00"), parse_money("10.10"));
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_at_effective") {
      update_bbo_price(
        fixture, simulator, parse_money("10.02"), parse_money("10.03"));
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("10.03"));
    }
  }

  TEST_CASE("submit_zero_quantity") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields =
      make_limit_order_fields(ABX, Side::BID, 0, parse_money("0.99"));
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::REJECTED);
  }

  TEST_CASE("submit_negative_quantity") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = TickerOrderSimulator(
      fixture.m_market_data_client, ABX, std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())),
      fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields =
      make_limit_order_fields(ABX, Side::BID, -100, parse_money("0.99"));
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PENDING_NEW);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::REJECTED);
  }

  TEST_CASE("time_and_sale_bid_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_above_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("1.00"), 100);
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("no_fill_at_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("no_fill_without_quantity") {
      publish_time_and_sale(fixture, simulator, parse_money("0.98"), 0);
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_below_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("0.98"), 100);
      auto report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("0.99"));
      REQUIRE(report.m_last_quantity == 100);
      REQUIRE(report.m_timestamp ==
        fixture.m_environment.get_time_environment().get_time());
    }
  }

  TEST_CASE("time_and_sale_ask_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::ASK, 100, parse_money("1.02"));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_below_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("1.01"), 100);
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("no_fill_at_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("1.02"), 100);
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_above_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("1.03"), 100);
      auto report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("1.02"));
      REQUIRE(report.m_last_quantity == 100);
    }

    SUBCASE("partial_fill_above_order_price") {
      publish_time_and_sale(fixture, simulator, parse_money("1.03"), 40);
      auto report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
      REQUIRE(report.m_last_price == parse_money("1.02"));
      REQUIRE(report.m_last_quantity == 40);
      publish_time_and_sale(fixture, simulator, parse_money("1.03"), 100);
      report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_price == parse_money("1.02"));
      REQUIRE(report.m_last_quantity == 60);
    }
  }

  TEST_CASE("time_and_sale_partial_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 1000, parse_money("0.99"));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 300);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 300);
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 300);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_quantity == 300);
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 900);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 400);
  }

  TEST_CASE("bbo_fill_after_partial_time_and_sale_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 1000, parse_money("0.99"));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 400);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_quantity == 400);
    update_bbo_price(
      fixture, simulator, parse_money("0.97"), parse_money("0.98"));
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.98"));
    REQUIRE(report.m_last_quantity == 600);
  }

  TEST_CASE("time_and_sale_fills_orders_by_price_then_time") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto first = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"));
    auto second = submit_limit_order(
      fixture, simulator, 2, Side::BID, 100, parse_money("0.99"));
    auto third = submit_limit_order(
      fixture, simulator, 3, Side::BID, 100, parse_money("1.00"));
    auto first_reports = monitor_reports(first);
    auto second_reports = monitor_reports(second);
    auto third_reports = monitor_reports(third);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 250);
    auto report = third_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("1.00"));
    REQUIRE(report.m_last_quantity == 100);
    report = first_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 100);
    report = second_reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 50);
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 50);
    report = second_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 50);
  }

  TEST_CASE("time_and_sale_fills_ask_orders_by_price_then_time") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto first = submit_limit_order(
      fixture, simulator, 1, Side::ASK, 100, parse_money("1.02"));
    auto second = submit_limit_order(
      fixture, simulator, 2, Side::ASK, 100, parse_money("1.02"));
    auto third = submit_limit_order(
      fixture, simulator, 3, Side::ASK, 100, parse_money("1.01"));
    auto first_reports = monitor_reports(first);
    auto second_reports = monitor_reports(second);
    auto third_reports = monitor_reports(third);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("1.03"), 250);
    auto report = third_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("1.01"));
    REQUIRE(report.m_last_quantity == 100);
    report = first_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("1.02"));
    REQUIRE(report.m_last_quantity == 100);
    report = second_reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_price == parse_money("1.02"));
    REQUIRE(report.m_last_quantity == 50);
    publish_time_and_sale(fixture, simulator, parse_money("1.03"), 50);
    report = second_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 50);
  }

  TEST_CASE("update_reduces_remaining_quantity") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 1000, parse_money("0.99"));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    auto fill = ExecutionReport();
    fill.m_id = order->get_info().m_id;
    fill.m_status = OrderStatus::PARTIALLY_FILLED;
    fill.m_last_quantity = 400;
    fill.m_last_price = parse_money("0.99");
    simulator.update(order, fill);
    fixture.m_reports->flush();
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_quantity == 400);
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 1000);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 600);
  }

  TEST_CASE("time_and_sale_condition") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));

    SUBCASE("no_fill_on_irregular_condition") {
      publish_time_and_sale(fixture, simulator, parse_money("0.98"), 100, "X");
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("no_fill_on_empty_condition") {
      publish_time_and_sale(fixture, simulator, parse_money("0.98"), 100, "");
      REQUIRE(!reports->try_pop());
    }

    SUBCASE("fill_on_regular_condition") {
      publish_time_and_sale(fixture, simulator, parse_money("0.98"), 100, "@");
      auto report = reports->pop();
      REQUIRE(report.m_status == OrderStatus::FILLED);
      REQUIRE(report.m_last_quantity == 100);
    }
  }

  TEST_CASE("time_and_sale_ignores_market_on_close_order") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_order(fixture, simulator, 1,
      OrderFields({}, ABX, CurrencyId::NONE, OrderType::LIMIT, Side::BID, {},
        100, parse_money("0.99"), TimeInForce(TimeInForce::Type::MOC), {}));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 100);
    REQUIRE(!reports->try_pop());
  }

  TEST_CASE("time_and_sale_ignores_pegged_order") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("10.00"), parse_money("10.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_order(fixture, simulator, 1,
      make_pegged_order_fields(ABX, Side::ASK, 100, Money::ZERO, Money::ZERO));
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("10.50"), 100);
    REQUIRE(!reports->try_pop());
  }

  TEST_CASE("recover_partially_filled_order") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto info = OrderInfo();
    info.m_fields =
      make_limit_order_fields(ABX, Side::BID, 1000, parse_money("0.99"));
    info.m_id = 1;
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto pending_new = ExecutionReport(info.m_id, info.m_timestamp);
    auto accepted =
      make_update(pending_new, OrderStatus::NEW, info.m_timestamp);
    auto partial_fill =
      make_update(accepted, OrderStatus::PARTIALLY_FILLED, info.m_timestamp);
    partial_fill.m_last_quantity = 400;
    partial_fill.m_last_price = parse_money("0.99");
    auto execution_reports =
      std::vector<ExecutionReport>({pending_new, accepted, partial_fill});
    auto order =
      std::make_shared<PrimitiveOrder>(OrderRecord(info, execution_reports));
    simulator.recover(order);
    fixture.m_reports->flush();
    auto reports = std::make_shared<Queue<ExecutionReport>>();
    order->get_publisher().monitor(reports);
    REQUIRE(reports->pop().m_status == OrderStatus::PENDING_NEW);
    REQUIRE(reports->pop().m_status == OrderStatus::NEW);
    REQUIRE(reports->pop().m_status == OrderStatus::PARTIALLY_FILLED);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.98"), 1000);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 600);
  }

  TEST_CASE("at_price_fill_at_front_of_queue") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("at_price_no_fill_behind_queue") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::BID,
      parse_money("0.99"), 500);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    REQUIRE(!reports->try_pop());
  }

  TEST_CASE("book_quote_decrease_advances_queue") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::BID,
      parse_money("0.99"), 500);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::BID,
      parse_money("0.99"), 200);
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    REQUIRE(!reports->try_pop());
    publish_book_quote(
      fixture, simulator, "A", Venues::TSX, Side::BID, parse_money("0.99"), 0);
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("book_quote_increase_does_not_advance_queue") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::BID,
      parse_money("0.99"), 500);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_book_quote(fixture, simulator, "B", Venues::TSX, Side::BID,
      parse_money("0.99"), 500);
    publish_book_quote(
      fixture, simulator, "B", Venues::TSX, Side::BID, parse_money("0.99"), 0);
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    REQUIRE(!reports->try_pop());
    publish_book_quote(
      fixture, simulator, "A", Venues::TSX, Side::BID, parse_money("0.99"), 0);
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("at_price_requires_matching_market_center") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(
      fixture, simulator, parse_money("0.99"), 100, "@", "ALP");
    REQUIRE(!reports->try_pop());
    publish_time_and_sale(
      fixture, simulator, parse_money("0.99"), 100, "@", "TSE");
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("queue_counts_only_the_destination_venue") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::BID,
      parse_money("0.99"), 500);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "ALPHA");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(
      fixture, simulator, parse_money("0.99"), 100, "@", "ALP");
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("queue_ignores_other_prices_and_sides") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::BID,
      parse_money("0.98"), 500);
    publish_book_quote(fixture, simulator, "B", Venues::TSX, Side::ASK,
      parse_money("0.99"), 500);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("at_price_ignores_unmapped_destination") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("0.99"), "MOE");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 100);
    REQUIRE(!reports->try_pop());
  }

  TEST_CASE("at_price_partial_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::BID, 1000, parse_money("0.99"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 300);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 300);
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 800);
    report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 700);
  }

  TEST_CASE("at_price_ask_fill") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    publish_book_quote(fixture, simulator, "A", Venues::TSX, Side::ASK,
      parse_money("1.02"), 500);
    auto order = submit_limit_order(
      fixture, simulator, 1, Side::ASK, 100, parse_money("1.02"), "TSX");
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("1.02"), 100);
    REQUIRE(!reports->try_pop());
    publish_book_quote(
      fixture, simulator, "A", Venues::TSX, Side::ASK, parse_money("1.02"), 0);
    publish_time_and_sale(fixture, simulator, parse_money("1.02"), 100);
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("1.02"));
    REQUIRE(report.m_last_quantity == 100);
  }

  TEST_CASE("through_orders_allocate_before_at_price_orders") {
    auto fixture = Fixture();
    fixture.m_environment.update_bbo_price(
      ABX, parse_money("1.00"), parse_money("1.01"));
    auto simulator = make_simulator(fixture);
    auto through = submit_limit_order(
      fixture, simulator, 1, Side::BID, 100, parse_money("1.00"), "TSX");
    auto resting = submit_limit_order(
      fixture, simulator, 2, Side::BID, 100, parse_money("0.99"), "TSX");
    auto through_reports = monitor_reports(through);
    auto resting_reports = monitor_reports(resting);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(fixture, simulator, parse_money("0.99"), 150);
    auto report = through_reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_price == parse_money("1.00"));
    REQUIRE(report.m_last_quantity == 100);
    report = resting_reports->pop();
    REQUIRE(report.m_status == OrderStatus::PARTIALLY_FILLED);
    REQUIRE(report.m_last_price == parse_money("0.99"));
    REQUIRE(report.m_last_quantity == 50);
  }

  TEST_CASE("destination_venue_follows_listing_venue") {
    auto fixture = Fixture();
    auto ticker = parse_ticker("XYZ.TSXV");
    fixture.m_environment.update_bbo_price(
      ticker, parse_money("1.00"), parse_money("1.01"));
    auto simulator = TickerOrderSimulator(fixture.m_market_data_client, ticker,
      std::make_unique<TestTimeClient>(
        Ref(fixture.m_environment.get_time_environment())), fixture.m_reports);
    auto info = OrderInfo();
    info.m_fields = make_limit_order_fields(
      ticker, CurrencyId::NONE, Side::BID, "TSX", 100, parse_money("0.99"));
    info.m_id = 1;
    info.m_timestamp = fixture.m_environment.get_time_environment().get_time();
    auto order = std::make_shared<PrimitiveOrder>(info);
    simulator.submit(order);
    fixture.m_reports->flush();
    auto reports = monitor_reports(order);
    fixture.m_environment.advance(minutes(1));
    publish_time_and_sale(
      fixture, simulator, parse_money("0.99"), 100, "@", "TSE");
    REQUIRE(!reports->try_pop());
    publish_time_and_sale(
      fixture, simulator, parse_money("0.99"), 100, "@", "CDX");
    auto report = reports->pop();
    REQUIRE(report.m_status == OrderStatus::FILLED);
    REQUIRE(report.m_last_quantity == 100);
  }
}
