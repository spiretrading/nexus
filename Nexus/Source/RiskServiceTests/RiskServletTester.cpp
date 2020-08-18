#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
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
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;

namespace {
  using TestServletContainer = TestAuthenticatedServiceProtocolServletContainer<
    MetaRiskServlet<VirtualAdministrationClient*,
    std::unique_ptr<VirtualMarketDataClient>,
    std::unique_ptr<VirtualOrderExecutionClient>, TriggerTimer,
    std::shared_ptr<FixedTimeClient>, LocalRiskDataStore*>>;

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    optional<AdministrationServiceTestEnvironment>
      m_administrationServiceEnvironment;
    optional<MarketDataServiceTestEnvironment> m_marketDataServiceEnvironment;
    optional<OrderExecutionServiceTestEnvironment>
      m_orderExecutionServiceEnvironment;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts;
    LocalRiskDataStore m_dataStore;
    optional<TestServletContainer> m_container;

    Fixture()
        : m_accounts(std::make_shared<Queue<DirectoryEntry>>()) {
      m_serviceLocatorEnvironment.Open();
      m_uidServiceEnvironment.Open();
      auto serviceLocatorClient =
        std::shared_ptr(m_serviceLocatorEnvironment.BuildClient());
      serviceLocatorClient->SetCredentials("root", "");
      serviceLocatorClient->Open();
      m_administrationServiceEnvironment.emplace(serviceLocatorClient);
      m_administrationServiceEnvironment->Open();
      auto administrationClient = std::shared_ptr(
        m_administrationServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)));
      m_marketDataServiceEnvironment.emplace(serviceLocatorClient,
        administrationClient);
      m_marketDataServiceEnvironment->Open();
      m_orderExecutionServiceEnvironment.emplace(GetDefaultMarketDatabase(),
        GetDefaultDestinationDatabase(), serviceLocatorClient,
        m_uidServiceEnvironment.BuildClient(), administrationClient);
      m_orderExecutionServiceEnvironment->Open();
      auto serverConnection = std::make_shared<TestServerConnection>();
      auto exchangeRates = std::vector<ExchangeRate>();
      m_container.emplace(Initialize(serviceLocatorClient,
        Initialize(m_accounts, &*administrationClient,
        m_marketDataServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)),
        m_orderExecutionServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)),
        [] {
          return std::make_unique<TriggerTimer>();
        }, std::make_shared<FixedTimeClient>(), &m_dataStore, exchangeRates,
        GetDefaultMarketDatabase(), GetDefaultDestinationDatabase())),
        serverConnection, factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
    }
  };
}

TEST_SUITE("RiskServlet") {
  TEST_CASE_FIXTURE(Fixture, "publishing") {
  }
}
