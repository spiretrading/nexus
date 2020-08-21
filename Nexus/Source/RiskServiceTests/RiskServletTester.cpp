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
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::RiskService;

namespace {
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  using TestServletContainer = TestAuthenticatedServiceProtocolServletContainer<
    MetaRiskServlet<VirtualAdministrationClient*,
    std::unique_ptr<VirtualMarketDataClient>,
    std::unique_ptr<VirtualOrderExecutionClient>, TriggerTimer,
    std::shared_ptr<FixedTimeClient>, LocalRiskDataStore*>>;

  struct Client {
    std::unique_ptr<VirtualServiceLocatorClient> m_serviceLocatorClient;
    std::unique_ptr<VirtualOrderExecutionClient> m_orderExecutionClient;
  };

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    optional<AdministrationServiceTestEnvironment>
      m_administrationServiceEnvironment;
    optional<MarketDataServiceTestEnvironment> m_marketDataServiceEnvironment;
    MockOrderExecutionDriver m_driver;
    optional<OrderExecutionServiceTestEnvironment>
      m_orderExecutionServiceEnvironment;
    std::shared_ptr<Queue<DirectoryEntry>> m_accounts;
    std::shared_ptr<Queue<PrimitiveOrder*>> m_orders;
    LocalRiskDataStore m_dataStore;
    optional<TestServletContainer> m_container;
    std::shared_ptr<VirtualAdministrationClient> m_administrationClient;

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
      m_administrationServiceEnvironment->MakeAdministrator(
        DirectoryEntry::GetRootAccount());
      m_administrationClient = std::shared_ptr(
        m_administrationServiceEnvironment->BuildClient(
        Ref(*serviceLocatorClient)));
      m_marketDataServiceEnvironment.emplace(serviceLocatorClient,
        m_administrationClient);
      m_marketDataServiceEnvironment->Open();
      m_orderExecutionServiceEnvironment.emplace(GetDefaultMarketDatabase(),
        GetDefaultDestinationDatabase(), serviceLocatorClient,
        m_uidServiceEnvironment.BuildClient(), m_administrationClient,
        MakeVirtualTimeClient(std::make_unique<FixedTimeClient>()),
        MakeVirtualOrderExecutionDriver(&m_driver));
      m_orders = std::make_shared<Queue<PrimitiveOrder*>>();
      m_driver.GetPublisher().Monitor(m_orders);
      m_orderExecutionServiceEnvironment->Open();
      auto serverConnection = std::make_shared<TestServerConnection>();
      auto exchangeRates = std::vector<ExchangeRate>();
      m_container.emplace(Initialize(serviceLocatorClient,
        Initialize(m_accounts, &*m_administrationClient,
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

    Client MakeClient(std::string name) {
      auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount(name,
        "1234", DirectoryEntry::GetStarDirectory());
      m_administrationClient->StoreRiskParameters(account,
        RiskParameters(DefaultCurrencies::USD(), 100000 * Money::ONE,
        RiskState::Type::ACTIVE, 100 * Money::ONE, 1, minutes(1)));
      m_administrationClient->StoreRiskState(account, RiskState::Type::ACTIVE);
      auto client = Client();
      client.m_serviceLocatorClient = m_serviceLocatorEnvironment.BuildClient();
      client.m_serviceLocatorClient->SetCredentials(name, "1234");
      client.m_serviceLocatorClient->Open();
      client.m_orderExecutionClient =
        m_orderExecutionServiceEnvironment->BuildClient(
        Ref(*client.m_serviceLocatorClient));
      client.m_orderExecutionClient->Open();
      return client;
    }
  };
}

TEST_SUITE("RiskServlet") {
  TEST_CASE_FIXTURE(Fixture, "reset_region") {
    auto client = MakeClient("simba");
    m_accounts->Push(client.m_serviceLocatorClient->GetAccount());
    auto& sentOrder = client.m_orderExecutionClient->Submit(
      OrderFields::BuildLimitOrder(XIU, Side::BID, 200, Money::ONE));
    auto receivedOrder = m_orders->Pop();
    Fill(*receivedOrder, 100);
  }
}
