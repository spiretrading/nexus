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
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto TST = Security("TST", TSX);
  using TestPortfolio = Portfolio<TrueAverageBookkeeper>;

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
    m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(Quote(Money::ONE, 100, Side::BID),
        Quote(2 * Money::ONE, 100, Side::ASK),
        time_from_string("2024-07-21 09:30:00.000")), TST));
    }
  };
}

TEST_SUITE("PortfolioController") {
  TEST_CASE("empty_orders") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    order_queue->Break();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = *controller.get_publisher().GetSnapshot();
    REQUIRE(updates->get_security_entries().empty());
    REQUIRE(updates->get_unrealized_profit_and_losses().empty());
  }

  TEST_CASE("single_order_lifecycle") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->Push(order);
    order_queue->Break();
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto update = updates->Pop();
    REQUIRE(update.m_security_inventory.m_position.m_security == TST);
    REQUIRE(update.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(
      update.m_security_inventory.m_position.m_cost_basis == 100 * Money::ONE);
  }

  TEST_CASE("multiple_orders_multiple_securities") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto security1 = Security("AAA", TSX);
    auto security2 = Security("BBB", TSX);
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(Quote(Money::ONE, 100, Side::BID),
        Quote(2 * Money::ONE, 100, Side::ASK), timestamp), security1));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(Quote(Money::ONE, 100, Side::BID),
        Quote(2 * Money::ONE, 100, Side::ASK), timestamp), security2));
    auto fields1 =
      make_limit_order_fields(security1, CAD, Side::BID, "TSX", 50, Money::ONE);
    auto info1 = OrderInfo(fields1, 1, false, timestamp);
    auto order1 = std::make_shared<PrimitiveOrder>(info1);
    order_queue->Push(order1);
    accept(*order1, timestamp + seconds(1));
    fill(*order1, 50, timestamp + seconds(2));
    auto update1 = updates->Pop();
    REQUIRE(update1.m_security_inventory.m_position.m_security == security1);
    REQUIRE(update1.m_security_inventory.m_position.m_quantity == 50);
    REQUIRE(
      update1.m_security_inventory.m_position.m_cost_basis == 50 * Money::ONE);
    auto fields2 =
      make_limit_order_fields(security2, CAD, Side::BID, "TSX", 75, Money::ONE);
    auto info2 = OrderInfo(fields2, 2, false, timestamp);
    auto order2 = std::make_shared<PrimitiveOrder>(info2);
    order_queue->Push(order2);
    accept(*order2, timestamp + seconds(1));
    fill(*order2, 75, timestamp + seconds(2));
    order_queue->Break();
    auto update2 = updates->Pop();
    REQUIRE(update2.m_security_inventory.m_position.m_security == security2);
    REQUIRE(update2.m_security_inventory.m_position.m_quantity == 75);
    REQUIRE(
      update2.m_security_inventory.m_position.m_cost_basis == 75 * Money::ONE);
  }

  TEST_CASE("bbo_quote_update_triggers_portfolio_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->Push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->Break();
    auto initial_update = updates->Pop();
    auto new_bbo = BboQuote(Quote(2 * Money::ONE, 100, Side::BID),
      Quote(3 * Money::ONE, 100, Side::ASK),
      timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(new_bbo, TST));
    auto bbo_update = updates->Pop();
    REQUIRE(bbo_update.m_security_inventory.m_position.m_security == TST);
    REQUIRE(bbo_update.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(
      bbo_update.m_unrealized_security != initial_update.m_unrealized_security);
    REQUIRE(bbo_update.m_security_inventory.m_position.m_cost_basis ==
      100 * Money::ONE);
  }

  TEST_CASE("bbo_quote_no_change_no_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->Push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->Break();
    auto initial_update = updates->Pop();
    auto unchanged_bbo = BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(2 * Money::ONE, 100, Side::ASK), timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(unchanged_bbo, TST));
    auto changed_bbo = BboQuote(Quote(2 * Money::ONE, 100, Side::BID),
      Quote(3 * Money::ONE, 100, Side::ASK), timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(changed_bbo, TST));
    auto final_update = updates->Pop();
    REQUIRE(final_update.m_security_inventory.m_position.m_security == TST);
    REQUIRE(final_update.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(final_update.m_unrealized_security == 100 * Money::ONE);
  }

  TEST_CASE("portfolio_initialization_with_existing_inventory") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto timestamp = time_from_string("2024-07-21 09:00:00.000");
    auto pre_fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 50, Money::CENT);
    auto pre_report = ExecutionReport(123, timestamp);
    pre_report.m_status = OrderStatus::FILLED;
    pre_report.m_last_quantity = 50;
    pre_report.m_last_price = Money::CENT;
    portfolio.update(pre_fields, pre_report);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->Push(order);
    order_queue->Break();
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto update1 = updates->Pop();
    REQUIRE(update1.m_security_inventory.m_position.m_security == TST);
    REQUIRE(update1.m_security_inventory.m_position.m_quantity == 50);
    REQUIRE(
      update1.m_security_inventory.m_position.m_cost_basis == 50 * Money::CENT);
    auto update2 = updates->Pop();
    REQUIRE(update2.m_security_inventory.m_position.m_security == TST);
    REQUIRE(update2.m_security_inventory.m_position.m_quantity == 150);
    REQUIRE(update2.m_security_inventory.m_position.m_cost_basis ==
      100 * Money::ONE + 50 * Money::CENT);
  }

  TEST_CASE("unrealized_pnl_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->Push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    order_queue->Break();
    auto initial_update = updates->Pop();
    auto new_bbo = BboQuote(Quote(3 * Money::ONE, 100, Side::BID),
      Quote(4 * Money::ONE, 100, Side::ASK),
      timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(new_bbo, TST));
    auto bbo_update = updates->Pop();
    REQUIRE(
      bbo_update.m_unrealized_security != initial_update.m_unrealized_security);
  }

  TEST_CASE("out_of_order_execution_reports") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
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
    order_queue->Push(order_a);
    order_queue->Push(order_b);
    order_queue->Push(order_c);
    accept(*order_a, timestamp + seconds(1));
    accept(*order_b, timestamp + seconds(2));
    accept(*order_c, timestamp + seconds(3));
    fill(*order_a, 100, timestamp + seconds(4));
    auto update1 = updates->Pop();
    REQUIRE(update1.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(update1.m_security_inventory.m_position.m_cost_basis ==
      100 * Money::CENT);
    fill(*order_c, 100, timestamp + seconds(5));
    auto update2 = updates->Pop();
    REQUIRE(update2.m_security_inventory.m_position.m_quantity == 0);
    REQUIRE(update2.m_security_inventory.m_position.m_cost_basis ==
      Money::ZERO);
    fill(*order_b, 100, timestamp + seconds(6));
    auto update3 = updates->Pop();
    REQUIRE(update3.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(update3.m_security_inventory.m_position.m_cost_basis ==
      200 * Money::CENT);
    REQUIRE(!updates->TryPop());
  }

  TEST_CASE("zero_position_bbo_update") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields_buy = make_limit_order_fields(
      Security("FOO", TSX), CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info_buy = OrderInfo(fields_buy, 1, false, timestamp);
    auto order_buy = std::make_shared<PrimitiveOrder>(info_buy);
    order_queue->Push(order_buy);
    accept(*order_buy, timestamp + seconds(1));
    fill(*order_buy, 100, timestamp + seconds(2));
    auto update1 = updates->Pop();
    REQUIRE(update1.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(update1.m_unrealized_security == Money::ZERO);
    auto fields_sell = make_limit_order_fields(
      Security("FOO", TSX), CAD, Side::ASK, "TSX", 100, Money::ONE);
    auto info_sell = OrderInfo(fields_sell, 2, false, timestamp);
    auto order_sell = std::make_shared<PrimitiveOrder>(info_sell);
    order_queue->Push(order_sell);
    accept(*order_sell, timestamp + seconds(3));
    fill(*order_sell, 100, timestamp + seconds(4));
    auto update2 = updates->Pop();
    REQUIRE(update2.m_security_inventory.m_position.m_quantity == 0);
    REQUIRE(update2.m_unrealized_security == Money::ZERO);
    auto new_bbo = BboQuote(Quote(2 * Money::ONE, 100, Side::BID),
      Quote(2 * Money::ONE, 100, Side::ASK), timestamp + seconds(5));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(new_bbo, Security("FOO", TSX)));
    REQUIRE(!updates->TryPop());
    auto fields_short = make_limit_order_fields(
      Security("FOO", TSX), CAD, Side::ASK, "TSX", 100, Money::ONE);
    auto info_short = OrderInfo(fields_short, 3, false, timestamp);
    auto order_short = std::make_shared<PrimitiveOrder>(info_short);
    order_queue->Push(order_short);
    accept(*order_short, timestamp + seconds(6));
    fill(*order_short, 100, timestamp + seconds(7));
    auto update3 = updates->Pop();
    REQUIRE(update3.m_security_inventory.m_position.m_quantity == -100);
  }

  TEST_CASE("empty_snapshot") {
    auto fixture = Fixture();
    auto portfolio = [&] {
      auto inventories = std::vector{Inventory(TST, CAD)};
      auto bookkeeper = TestPortfolio::Bookkeeper(inventories);
      return TestPortfolio(DEFAULT_VENUES);
    }();
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    REQUIRE(!updates->TryPop());
  }

  TEST_CASE("no_bbo_available") {
    auto fixture = Fixture();
    auto portfolio = TestPortfolio(DEFAULT_VENUES);
    auto order_queue = std::make_shared<Queue<std::shared_ptr<const Order>>>();
    auto controller = PortfolioController(
      &portfolio, fixture.m_market_data_client, order_queue);
    auto updates = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_publisher().Monitor(updates);
    auto timestamp = time_from_string("2024-07-21 10:00:00.000");
    auto fields =
      make_limit_order_fields(TST, CAD, Side::BID, "TSX", 100, Money::ONE);
    auto info = OrderInfo(fields, 1, false, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    order_queue->Push(order);
    accept(*order, timestamp + seconds(1));
    fill(*order, 100, timestamp + seconds(2));
    auto update1 = updates->Pop();
    REQUIRE(update1.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(update1.m_unrealized_security == Money::ZERO);
    auto new_bbo = BboQuote(Quote(2 * Money::ONE, 100, Side::BID),
      Quote(2 * Money::ONE, 100, Side::ASK), timestamp + seconds(3));
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(new_bbo, TST));
    auto update2 = updates->Pop();
    REQUIRE(update2.m_security_inventory.m_position.m_quantity == 100);
    REQUIRE(update2.m_unrealized_security == 100 * Money::ONE);
  }
}
