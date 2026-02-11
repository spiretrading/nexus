#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  using TestPortfolio = Portfolio<TrueAverageBookkeeper>;
  const auto TST = parse_ticker("TST.TSX");

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;
    MarketDataClient m_market_data_client;

    Fixture()
      : m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)),
        m_market_data_environment(
          make_market_data_service_test_environment(
            m_service_locator_environment, m_administration_environment)),
        m_market_data_client(m_market_data_environment.get_registry_client()) {
      m_market_data_environment.get_feed_client().publish(TickerBboQuote(
        BboQuote(make_bid(Money::ONE, 100), make_ask(2 * Money::ONE, 100),
          time_from_string("2024-07-21 09:30:00.000")), TST));
    }
  };
}

TEST_SUITE("PortfolioController") {
  TEST_CASE("empty_orders") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    order_queue->close();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = *controller.get_publisher().get_snapshot();
    REQUIRE(updates->get_entries().empty());
    REQUIRE(updates->get_unrealized_profit_and_losses().empty());
  }

  TEST_CASE("single_order_lifecycle") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->push(order);
    order_queue->close();
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto update = updates->pop();
    REQUIRE(update.m_inventory.m_position.m_ticker == TST);
    REQUIRE(update.m_inventory.m_position.m_quantity == 100);
    REQUIRE(update.m_inventory.m_position.m_cost_basis == 100 * Money::ONE);
  }

  TEST_CASE("multiple_orders_multiple_securities") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto ticker1 = parse_ticker("AAA.TSX");
    auto ticker2 = parse_ticker("BBB.TSX");
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(BboQuote(
        make_bid(Money::ONE, 100), make_ask(2 * Money::ONE, 100), timestamp),
        ticker1));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(BboQuote(
        make_bid(Money::ONE, 100), make_ask(2 * Money::ONE, 100), timestamp),
        ticker2));
    auto fields1 =
      make_limit_order_fields(ticker1, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info1 = OrderInfo(fields1, 1, false, timestamp);
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    order_queue->push(order1);
    accept(*order1, timestamp + seconds(1));
    fill(*order1, 50, timestamp + seconds(2));
    auto update1 = updates->pop();
    REQUIRE(update1.m_inventory.m_position.m_ticker == ticker1);
    REQUIRE(update1.m_inventory.m_position.m_quantity == 50);
    REQUIRE(update1.m_inventory.m_position.m_cost_basis == 50 * Money::ONE);
    auto fields2 =
      make_limit_order_fields(ticker2, CAD, Side::BID, "TSX", 75, Money::ONE);
    auto info2 = OrderInfo(fields2, 2, false, timestamp);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    order_queue->push(order2);
    accept(*order2, timestamp + seconds(1));
    fill(*order2, 75, timestamp + seconds(2));
    order_queue->close();
    auto update2 = updates->pop();
    REQUIRE(update2.m_inventory.m_position.m_ticker == ticker2);
    REQUIRE(update2.m_inventory.m_position.m_quantity == 75);
    REQUIRE(update2.m_inventory.m_position.m_cost_basis == 75 * Money::ONE);
  }

  TEST_CASE("bbo_quote_update_triggers_portfolio_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->close();
    auto initial_update = updates->pop();
    auto new_bbo =
      BboQuote(make_bid(2 * Money::ONE, 100), make_ask(3 * Money::ONE, 100),
        timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(new_bbo, TST));
    auto bbo_update = updates->pop();
    REQUIRE(bbo_update.m_inventory.m_position.m_ticker == TST);
    REQUIRE(bbo_update.m_inventory.m_position.m_quantity == 100);
    REQUIRE(bbo_update.m_unrealized != initial_update.m_unrealized);
    REQUIRE(bbo_update.m_inventory.m_position.m_cost_basis == 100 * Money::ONE);
  }

  TEST_CASE("bbo_quote_no_change_no_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->close();
    auto initial_update = updates->pop();
    auto unchanged_bbo =
      BboQuote(make_bid(Money::ONE, 100), make_ask(2 * Money::ONE, 100),
        timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(unchanged_bbo, TST));
    auto changed_bbo =
      BboQuote(make_bid(2 * Money::ONE, 100), make_ask(3 * Money::ONE, 100),
        timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(changed_bbo, TST));
    auto final_update = updates->pop();
    REQUIRE(final_update.m_inventory.m_position.m_ticker == TST);
    REQUIRE(final_update.m_inventory.m_position.m_quantity == 100);
    REQUIRE(final_update.m_unrealized == 100 * Money::ONE);
  }

  TEST_CASE("portfolio_initialization_with_existing_inventory") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto timestamp = time_from_string("2024-07-21 09:00:00.000");
    auto pre_fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 50, Money::CENT);
    auto pre_report = ExecutionReport(123, timestamp);
    pre_report.m_status = OrderStatus::FILLED;
    pre_report.m_last_quantity = 50;
    pre_report.m_last_price = Money::CENT;
    portfolio.update(pre_fields, pre_report);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->push(order);
    order_queue->close();
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto update1 = updates->pop();
    REQUIRE(update1.m_inventory.m_position.m_ticker == TST);
    REQUIRE(update1.m_inventory.m_position.m_quantity == 50);
    REQUIRE(update1.m_inventory.m_position.m_cost_basis == 50 * Money::CENT);
    auto update2 = updates->pop();
    REQUIRE(update2.m_inventory.m_position.m_ticker == TST);
    REQUIRE(update2.m_inventory.m_position.m_quantity == 150);
    REQUIRE(update2.m_inventory.m_position.m_cost_basis ==
      100 * Money::ONE + 50 * Money::CENT);
  }

  TEST_CASE("unrealized_pnl_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->close();
    auto initial_update = updates->pop();
    auto new_bbo =
      BboQuote(make_bid(3 * Money::ONE, 100), make_ask(4 * Money::ONE, 100),
        timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(new_bbo, TST));
    auto bbo_update = updates->pop();
    REQUIRE(bbo_update.m_unrealized != initial_update.m_unrealized);
  }

  TEST_CASE("out_of_order_execution_reports") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields_a =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::CENT);
    auto info_a = OrderInfo(fields_a, 1, false, timestamp);
    auto order_a = std::make_shared<PrimitiveOrder>(info_a);
    auto fields_b =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, 2 * Money::CENT);
    auto info_b = OrderInfo(fields_b, 2, false, timestamp);
    auto order_b = std::make_shared<PrimitiveOrder>(info_b);
    auto fields_c =
      make_limit_order_fields(TST, CAD, Side::ASK, "TSX", 100, 3 * Money::CENT);
    auto info_c = OrderInfo(fields_c, 3, false, timestamp);
    auto order_c = std::make_shared<PrimitiveOrder>(info_c);
    order_queue->push(order_a);
    order_queue->push(order_b);
    order_queue->push(order_c);
    accept(*order_a, timestamp + seconds(1));
    accept(*order_b, timestamp + seconds(2));
    accept(*order_c, timestamp + seconds(3));
    fill(*order_a, 100, timestamp + seconds(4));
    auto update1 = updates->pop();
    REQUIRE(update1.m_inventory.m_position.m_quantity == 100);
    REQUIRE(update1.m_inventory.m_position.m_cost_basis == 100 * Money::CENT);
    fill(*order_c, 100, timestamp + seconds(5));
    auto update2 = updates->pop();
    REQUIRE(update2.m_inventory.m_position.m_quantity == 0);
    REQUIRE(update2.m_inventory.m_position.m_cost_basis == Money::ZERO);
    fill(*order_b, 100, timestamp + seconds(6));
    auto update3 = updates->pop();
    REQUIRE(update3.m_inventory.m_position.m_quantity == 100);
    REQUIRE(update3.m_inventory.m_position.m_cost_basis == 200 * Money::CENT);
    REQUIRE(!updates->try_pop());
  }

  TEST_CASE("zero_position_bbo_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields_buy = make_limit_order_fields(
      parse_ticker("FOO.TSX"), CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info_buy = OrderInfo(fields_buy, 1, false, timestamp);
    auto order_buy = std::make_shared<PrimitiveOrder>(info_buy);
    order_queue->push(order_buy);
    accept(*order_buy, timestamp + seconds(1));
    fill(*order_buy, 100, timestamp + seconds(2));
    auto update1 = updates->pop();
    REQUIRE(update1.m_inventory.m_position.m_quantity == 100);
    REQUIRE(update1.m_unrealized == Money::ZERO);
    auto fields_sell = make_limit_order_fields(
      parse_ticker("FOO.TSX"), CAD, Side::ASK, "TSX", 100, Money::ONE);
    auto info_sell = OrderInfo(fields_sell, 2, false, timestamp);
    auto order_sell = std::make_shared<PrimitiveOrder>(info_sell);
    order_queue->push(order_sell);
    accept(*order_sell, timestamp + seconds(3));
    fill(*order_sell, 100, timestamp + seconds(4));
    auto update2 = updates->pop();
    REQUIRE(update2.m_inventory.m_position.m_quantity == 0);
    REQUIRE(update2.m_unrealized == Money::ZERO);
    auto new_bbo =
      BboQuote(make_bid(2 * Money::ONE, 100), make_ask(2 * Money::ONE, 100),
        timestamp + seconds(5));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(new_bbo, parse_ticker("FOO.TSX")));
    REQUIRE(!updates->try_pop());
    auto fields_short = make_limit_order_fields(
      parse_ticker("FOO.TSX"), CAD, Side::ASK, "TSX", 100, Money::ONE);
    auto info_short = OrderInfo(fields_short, 3, false, timestamp);
    auto order_short = std::make_shared<PrimitiveOrder>(info_short);
    order_queue->push(order_short);
    accept(*order_short, timestamp + seconds(6));
    fill(*order_short, 100, timestamp + seconds(7));
    auto update3 = updates->pop();
    REQUIRE(update3.m_inventory.m_position.m_quantity == -100);
  }

  TEST_CASE("empty_snapshot") {
    auto fixture = Fixture();
    auto portfolio = [&] {
      auto inventories = std::vector{Inventory(TST, CAD)};
      auto bookkeeper = TestPortfolio::Bookkeeper(inventories);
      return TestPortfolio();
    }();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    REQUIRE(!updates->try_pop());
  }

  TEST_CASE("no_bbo_available") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    auto update1 = updates->pop();
    REQUIRE(update1.m_inventory.m_position.m_quantity == 100);
    REQUIRE(update1.m_unrealized == Money::ZERO);
    auto new_bbo =
      BboQuote(make_bid(2 * Money::ONE, 100), make_ask(2 * Money::ONE, 100),
        timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      TickerBboQuote(new_bbo, TST));
    auto update2 = updates->pop();
    REQUIRE(update2.m_inventory.m_position.m_quantity == 100);
    REQUIRE(update2.m_unrealized == 100 * Money::ONE);
  }
}
