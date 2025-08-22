#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskController.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;
using namespace Nexus::Tests;

namespace {
  auto TSLA = Security("TSLA", NASDAQ);
  auto XIU = Security("XIU", TSX);

  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    UidServiceTestEnvironment m_uid_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;
    OrderExecutionServiceTestEnvironment m_order_execution_environment;
    TriggerTimer m_timer;
    FixedTimeClient m_time_client;
    LocalRiskDataStore m_data_store;
    ExchangeRateTable m_exchange_rates;
    DirectoryEntry m_trader_account;
    optional<ServiceLocatorClientBox> m_service_locator;
    optional<AdministrationClient> m_administration_client;
    optional<MarketDataClient> m_market_data_client;
    optional<OrderExecutionClient> m_service_order_execution_client;
    optional<OrderExecutionClient> m_trader_order_execution_client;
    std::shared_ptr<Queue<std::shared_ptr<PrimitiveOrder>>> m_order_submissions;

    Fixture()
        : m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_market_data_environment(make_market_data_service_test_environment(
            m_service_locator_environment, m_administration_environment)),
          m_order_execution_environment(
            make_order_execution_service_test_environment(
              m_service_locator_environment, m_uid_environment,
              m_administration_environment)),
          m_time_client(time_from_string("2025-07-14 6:23:00:00")) {
      auto servlet_account =
        m_service_locator_environment.GetRoot().MakeAccount("risk_service", "",
          DirectoryEntry::GetStarDirectory());
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator =
        m_service_locator_environment.MakeClient("risk_service", "");
      grant_all_entitlements(
        m_administration_environment, m_service_locator->GetAccount());
      m_administration_client =
        m_administration_environment.make_client(*m_service_locator);
      m_trader_account =
        m_service_locator_environment.GetRoot().MakeAccount("trader", "",
          DirectoryEntry::GetStarDirectory());
      m_administration_environment.get_client().store(m_trader_account,
        RiskParameters(USD, 100000 * Money::ONE, RiskState::Type::ACTIVE,
          2 * Money::ONE, minutes(10)));
      m_administration_environment.get_client().store(
        m_trader_account, RiskState::Type::ACTIVE);
      m_market_data_client =
        m_market_data_environment.make_registry_client(*m_service_locator);
      m_service_order_execution_client =
        m_order_execution_environment.make_client(*m_service_locator);
      auto trader_service_locator =
        m_service_locator_environment.MakeClient("trader", "");
      m_trader_order_execution_client =
        m_order_execution_environment.make_client(trader_service_locator);
      m_order_submissions =
        std::make_shared<Queue<std::shared_ptr<PrimitiveOrder>>>();
      m_order_execution_environment.get_driver().as<MockOrderExecutionDriver>().
        get_publisher().Monitor(m_order_submissions);
      m_market_data_environment.get_feed_client().publish(
        SecurityBboQuote(BboQuote(Quote(parse_money("1.00"), 100, Side::BID),
          Quote(parse_money("1.01"), 100, Side::ASK), m_time_client.GetTime()),
          TSLA));
    }
  };
}

TEST_SUITE("RiskController") {
  TEST_CASE("single_security_no_position") {
    auto fixture = Fixture();
    auto controller = RiskController(fixture.m_trader_account,
      *fixture.m_administration_client, *fixture.m_market_data_client,
      *fixture.m_service_order_execution_client, &fixture.m_timer,
      &fixture.m_time_client, &fixture.m_data_store, fixture.m_exchange_rates,
      DEFAULT_VENUES, DEFAULT_DESTINATIONS);
    auto state = std::make_shared<Queue<RiskState>>();
    controller.get_risk_state_publisher().Monitor(state);
    REQUIRE(state->Pop() == RiskState::Type::ACTIVE);
    auto portfolio = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_portfolio_publisher().Monitor(portfolio);
    auto order = fixture.m_trader_order_execution_client->submit(
      make_market_order_fields(TSLA, Side::BID, 100));
    auto received_order = fixture.m_order_submissions->Pop();
    accept(*received_order);
    fill(*received_order, parse_money("1.01"), 100);
    auto update = portfolio->Pop();
    REQUIRE(update.m_unrealized_security == -Money::ONE);
    REQUIRE(update.m_unrealized_currency == -Money::ONE);
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(Quote(parse_money("0.99"), 100, Side::BID),
        Quote(parse_money("1.00"), 100, Side::ASK),
        fixture.m_time_client.GetTime()), TSLA));
    REQUIRE((state->Pop().m_type == RiskState::Type::CLOSE_ORDERS));
    auto new_parameters = RiskParameters(USD, 100000 * Money::ONE,
      RiskState::Type::ACTIVE, 1000 * Money::ONE, minutes(10));
    fixture.m_administration_client->store(
      fixture.m_trader_account, new_parameters);
    REQUIRE(state->Pop().m_type == RiskState::Type::ACTIVE);
  }

  TEST_CASE("single_security_existing_position") {
    auto fixture = Fixture();
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories.push_back(Inventory(Position(TSLA, USD,
      200, 200 * (99 * Money::CENT)), Money::ZERO, Money::ZERO, 200, 1));
    fixture.m_data_store.store(fixture.m_trader_account, snapshot);
    auto controller = RiskController(fixture.m_trader_account,
      *fixture.m_administration_client, *fixture.m_market_data_client,
      *fixture.m_service_order_execution_client, &fixture.m_timer,
      &fixture.m_time_client, &fixture.m_data_store, fixture.m_exchange_rates,
      DEFAULT_VENUES, DEFAULT_DESTINATIONS);
    auto state = std::make_shared<Queue<RiskState>>();
    controller.get_risk_state_publisher().Monitor(state);
    REQUIRE(state->Pop() == RiskState::Type::ACTIVE);
    auto portfolio = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_portfolio_publisher().Monitor(portfolio);
    auto update = portfolio->Pop();
    REQUIRE(update.m_security_inventory.m_position.m_security == TSLA);
    REQUIRE(update.m_security_inventory.m_position.m_currency == USD);
    REQUIRE(update.m_unrealized_security == 2 * Money::ONE);
    REQUIRE(update.m_unrealized_currency == 2 * Money::ONE);
    REQUIRE(update.m_security_inventory.m_position.m_quantity == 200);
    auto order = fixture.m_trader_order_execution_client->submit(
      make_market_order_fields(TSLA, Side::BID, 100));
    auto received_order = fixture.m_order_submissions->Pop();
    accept(*received_order);
    fill(*received_order, parse_money("0.99"), 100);
    update = portfolio->Pop();
    REQUIRE(update.m_security_inventory.m_position.m_security == TSLA);
    REQUIRE(update.m_security_inventory.m_position.m_quantity == 300);
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(Quote(parse_money("0.98"), 100, Side::BID),
        Quote(parse_money("0.99"), 100, Side::ASK),
        fixture.m_time_client.GetTime()), TSLA));
    REQUIRE(state->Pop().m_type == RiskState::Type::CLOSE_ORDERS);
    fixture.m_time_client.SetTime(
      fixture.m_time_client.GetTime() + minutes(10));
    fixture.m_timer.Trigger();
    REQUIRE(state->Pop().m_type == RiskState::Type::DISABLED);
  }
}
