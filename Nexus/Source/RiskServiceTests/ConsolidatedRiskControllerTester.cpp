#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/ConsolidatedRiskController.hpp"

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
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;

namespace {
  struct Fixture {
    ServiceLocatorTestEnvironment m_service_locator_environment;
    UidServiceTestEnvironment m_uid_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    MarketDataServiceTestEnvironment m_market_data_environment;
    OrderExecutionServiceTestEnvironment m_order_execution_environment;
    FixedTimeClient m_time_client;
    LocalRiskDataStore m_data_store;
    ExchangeRateTable m_exchange_rates;
    optional<ServiceLocatorClientBox> m_service_locator;
    optional<AdministrationClient> m_administration_client;
    optional<MarketDataClient> m_market_data_client;
    optional<OrderExecutionClient> m_service_order_execution_client;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts_queue;

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
          m_time_client(time_from_string("2025-07-14 6:23:00:00")),
          m_accounts_queue(std::make_shared<Queue<DirectoryEntry>>()) {
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
      m_market_data_client =
        m_market_data_environment.make_registry_client(*m_service_locator);
      m_service_order_execution_client =
        m_order_execution_environment.make_client(*m_service_locator);
    }
  };
}

TEST_SUITE("ConsolidatedRiskController") {
  TEST_CASE("add_account") {
    auto fixture = Fixture();
    auto timer_factory = [] {
      return std::make_unique<TriggerTimer>();
    };
    auto consolidated_controller = ConsolidatedRiskController(
      fixture.m_accounts_queue, &*fixture.m_administration_client,
      &*fixture.m_market_data_client,
      &*fixture.m_service_order_execution_client, timer_factory,
      &fixture.m_time_client, &fixture.m_data_store, fixture.m_exchange_rates,
      DEFAULT_VENUES, DEFAULT_DESTINATIONS);
    auto state_updates = std::make_shared<Queue<RiskStateEntry>>();
    consolidated_controller.get_risk_state_publisher().Monitor(state_updates);
    auto account1 = fixture.m_service_locator_environment.GetRoot().MakeAccount(
      "trader", "", DirectoryEntry::GetStarDirectory());
    fixture.m_administration_environment.get_client().store(account1,
      RiskParameters(USD, 100000 * Money::ONE, RiskState::Type::ACTIVE,
        2 * Money::ONE, minutes(10)));
    fixture.m_administration_environment.get_client().store(
      account1, RiskState::Type::ACTIVE);
    fixture.m_accounts_queue->Push(account1);
    auto state_entry = state_updates->Pop();
    REQUIRE(state_entry.m_key == account1);
    REQUIRE(state_entry.m_value == RiskState::Type::ACTIVE);
    auto account2 = fixture.m_service_locator_environment.GetRoot().MakeAccount(
      "trader2", "", DirectoryEntry::GetStarDirectory());
    fixture.m_administration_environment.get_client().store(account2,
      RiskParameters(USD, 50000 * Money::ONE, RiskState::Type::ACTIVE,
        1 * Money::ONE, minutes(5)));
    fixture.m_administration_environment.get_client().store(
      account2, RiskState::Type::ACTIVE);
    fixture.m_accounts_queue->Push(account2);
    state_entry = state_updates->Pop();
    REQUIRE(state_entry.m_key == account2);
    REQUIRE(state_entry.m_value == RiskState::Type::ACTIVE);
  }
}
