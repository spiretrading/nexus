#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
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
#include "Nexus/RiskService/RiskServlet.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
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
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaRiskServlet<AdministrationClient, MarketDataClient,
        OrderExecutionClient, TriggerTimer, FixedTimeClient*,
        LocalRiskDataStore*>>;
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
    std::shared_ptr<TestServerConnection> m_server_connection;
    optional<ServletContainer> m_container;

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
          m_accounts_queue(std::make_shared<Queue<DirectoryEntry>>()),
          m_server_connection(std::make_shared<TestServerConnection>()) {
      auto servlet_account =
        m_service_locator_environment.GetRoot().MakeAccount("risk_service", "",
          DirectoryEntry::GetStarDirectory());
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator =
        m_service_locator_environment.MakeClient("risk_service", "");
      auto entitlements = [&] {
        auto entitlements = std::vector<DirectoryEntry>();
        for(auto& entry : m_administration_environment.get_client().
            load_entitlements().get_entries()) {
          entitlements.push_back(entry.m_group_entry);
        }
        return entitlements;
      }();
      m_administration_environment.get_client().store_entitlements(
        m_service_locator->GetAccount(), entitlements);
      m_administration_client =
        m_administration_environment.make_client(*m_service_locator);
      m_market_data_client =
        m_market_data_environment.make_registry_client(*m_service_locator);
      m_service_order_execution_client =
        m_order_execution_environment.make_client(*m_service_locator);
      m_container.emplace(Initialize(*m_service_locator,
        Initialize(m_accounts_queue, *m_administration_client,
          *m_market_data_client, *m_service_order_execution_client,
          factory<std::unique_ptr<TriggerTimer>>(), &m_time_client,
          &m_data_store, m_exchange_rates, DEFAULT_VENUES,
          DEFAULT_DESTINATIONS)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
    }
  };
}

TEST_SUITE("RiskServlet") {
}
