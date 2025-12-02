#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskController.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto S32 = Security("S32", ASX);
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
    optional<ServiceLocatorClient> m_service_locator;
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
        m_service_locator_environment.get_root().make_account("risk_service",
          "", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator.emplace(
        m_service_locator_environment.make_client("risk_service", ""));
      grant_all_entitlements(
        m_administration_environment, m_service_locator->get_account());
      m_administration_client.emplace(
        m_administration_environment.make_client(Ref(*m_service_locator)));
      m_trader_account =
        m_service_locator_environment.get_root().make_account("trader", "",
          DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.get_client().store(m_trader_account,
        RiskParameters(AUD, 100000 * Money::ONE, RiskState::Type::ACTIVE,
          2 * Money::ONE, minutes(10)));
      m_administration_environment.get_client().store(
        m_trader_account, RiskState::Type::ACTIVE);
      m_market_data_client.emplace(
        m_market_data_environment.make_registry_client(
          Ref(*m_service_locator)));
      m_service_order_execution_client.emplace(
        m_order_execution_environment.make_client(Ref(*m_service_locator)));
      auto trader_service_locator =
        m_service_locator_environment.make_client("trader", "");
      m_trader_order_execution_client.emplace(
        m_order_execution_environment.make_client(Ref(trader_service_locator)));
      m_order_submissions =
        std::make_shared<Queue<std::shared_ptr<PrimitiveOrder>>>();
      m_order_execution_environment.get_driver().as<MockOrderExecutionDriver>().
        get_publisher().monitor(m_order_submissions);
      m_market_data_environment.get_feed_client().publish(
        SecurityBboQuote(BboQuote(make_bid(parse_money("1.00"), 100),
          make_ask(parse_money("1.01"), 100), m_time_client.get_time()), S32));
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
    controller.get_risk_state_publisher().monitor(state);
    REQUIRE(state->pop() == RiskState::Type::ACTIVE);
    auto portfolio = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_portfolio_publisher().monitor(portfolio);
    auto order = fixture.m_trader_order_execution_client->submit(
      make_market_order_fields(S32, Side::BID, 100));
    auto received_order = fixture.m_order_submissions->pop();
    accept(*received_order);
    fill(*received_order, parse_money("1.01"), 100);
    auto update = portfolio->pop();
    REQUIRE(update.m_unrealized_security == -Money::ONE);
    REQUIRE(update.m_unrealized_currency == -Money::ONE);
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(
        make_bid(parse_money("0.99"), 100), make_ask(parse_money("1.00"), 100),
        fixture.m_time_client.get_time()), S32));
    REQUIRE((state->pop().m_type == RiskState::Type::CLOSE_ORDERS));
    auto new_parameters = RiskParameters(AUD, 100000 * Money::ONE,
      RiskState::Type::ACTIVE, 1000 * Money::ONE, minutes(10));
    fixture.m_administration_client->store(
      fixture.m_trader_account, new_parameters);
    REQUIRE(state->pop().m_type == RiskState::Type::ACTIVE);
  }

  TEST_CASE("single_security_existing_position") {
    auto fixture = Fixture();
    auto snapshot = InventorySnapshot();
    snapshot.m_inventories.push_back(Inventory(Position(S32, AUD,
      200, 200 * (99 * Money::CENT)), Money::ZERO, Money::ZERO, 200, 1));
    fixture.m_data_store.store(fixture.m_trader_account, snapshot);
    auto controller = RiskController(fixture.m_trader_account,
      *fixture.m_administration_client, *fixture.m_market_data_client,
      *fixture.m_service_order_execution_client, &fixture.m_timer,
      &fixture.m_time_client, &fixture.m_data_store, fixture.m_exchange_rates,
      DEFAULT_VENUES, DEFAULT_DESTINATIONS);
    auto state = std::make_shared<Queue<RiskState>>();
    controller.get_risk_state_publisher().monitor(state);
    REQUIRE(state->pop() == RiskState::Type::ACTIVE);
    auto portfolio = std::make_shared<Queue<PortfolioUpdateEntry>>();
    controller.get_portfolio_publisher().monitor(portfolio);
    auto update = portfolio->pop();
    REQUIRE(update.m_security_inventory.m_position.m_security == S32);
    REQUIRE(update.m_security_inventory.m_position.m_currency == AUD);
    REQUIRE(update.m_unrealized_security == 2 * Money::ONE);
    REQUIRE(update.m_unrealized_currency == 2 * Money::ONE);
    REQUIRE(update.m_security_inventory.m_position.m_quantity == 200);
    auto order = fixture.m_trader_order_execution_client->submit(
      make_market_order_fields(S32, Side::BID, 100));
    auto received_order = fixture.m_order_submissions->pop();
    accept(*received_order);
    fill(*received_order, parse_money("0.99"), 100);
    update = portfolio->pop();
    REQUIRE(update.m_security_inventory.m_position.m_security == S32);
    REQUIRE(update.m_security_inventory.m_position.m_quantity == 300);
    fixture.m_market_data_environment.get_feed_client().publish(
      SecurityBboQuote(BboQuote(
        make_bid(parse_money("0.98"), 100), make_ask(parse_money("0.99"), 100),
        fixture.m_time_client.get_time()), S32));
    REQUIRE(state->pop().m_type == RiskState::Type::CLOSE_ORDERS);
    fixture.m_time_client.set(fixture.m_time_client.get_time() + minutes(10));
    fixture.m_timer.trigger();
    REQUIRE(state->pop().m_type == RiskState::Type::DISABLED);
  }
}
