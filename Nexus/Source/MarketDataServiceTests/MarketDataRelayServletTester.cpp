#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRelayServlet.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

namespace {
  const auto TST_A = Security("TST_A", DefaultMarkets::TSX(),
    DefaultCountries::CA());
  const auto TST_B = Security("TST_B", DefaultMarkets::TSX(),
    DefaultCountries::CA());

  using TestServletContainer =
    TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRelayServlet<MarketDataClientBox, AdministrationClientBox>>;
  using TestMarketDataClient =
    MarketDataClient<TestServiceProtocolClientBuilder>;

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    AdministrationServiceTestEnvironment m_administrationEnvironment;
    LocalHistoricalDataStore m_dataStore;
    MarketDataServiceTestEnvironment m_marketDataEnvironment;
    std::shared_ptr<TestServerConnection> m_relayServerConnection;
    optional<TestServletContainer> m_container;

    Fixture()
        : m_administrationEnvironment(MakeAdministrationServiceTestEnvironment(
            m_serviceLocatorEnvironment)),
          m_marketDataEnvironment(m_serviceLocatorEnvironment.GetRoot(),
            m_administrationEnvironment.GetClient(),
            HistoricalDataStoreBox(&m_dataStore)),
          m_relayServerConnection(std::make_shared<TestServerConnection>()) {
      m_dataStore.Store(SecurityInfo(TST_A, "Test A", "", 100));
      m_dataStore.Store(SecurityInfo(TST_B, "Test B", "", 100));
      auto entitlements = std::vector<DirectoryEntry>();
      auto entries =
        m_administrationEnvironment.GetClient().LoadEntitlements().GetEntries();
      for(auto& entitlement : entries) {
        entitlements.push_back(entitlement.m_groupEntry);
      }
      m_administrationEnvironment.GetClient().StoreEntitlements(
        m_serviceLocatorEnvironment.GetRoot().GetAccount(), entitlements);
      auto marketDataClientFactory = [this] {
        return std::make_unique<MarketDataClientBox>(MakeMarketDataClient());
      };
      m_container.emplace(Initialize(m_serviceLocatorEnvironment.GetRoot(),
        Initialize(seconds(100), marketDataClientFactory, 1, 1,
          m_administrationEnvironment.GetClient())), m_relayServerConnection,
        factory<std::unique_ptr<TriggerTimer>>());
    }

    MarketDataClientBox MakeMarketDataClient() {
      return m_marketDataEnvironment.MakeRegistryClient(
        m_serviceLocatorEnvironment.GetRoot());
    }

    std::unique_ptr<TestServiceProtocolClient> MakeMarketDataRelayClient(
        const std::string& username) {
      auto account = m_serviceLocatorEnvironment.GetRoot().MakeAccount(username,
        "1234", DirectoryEntry::GetStarDirectory());
      auto entitlements = std::vector<DirectoryEntry>();
      auto entries =
        m_administrationEnvironment.GetClient().LoadEntitlements().GetEntries();
      for(auto& entitlement : entries) {
        entitlements.push_back(entitlement.m_groupEntry);
      }
      m_administrationEnvironment.GetClient().StoreEntitlements(account,
        entitlements);
      auto serviceLocatorClient = m_serviceLocatorEnvironment.MakeClient(
        username, "1234");
      auto authenticator = SessionAuthenticator(serviceLocatorClient);
      auto protocolClient = std::make_unique<TestServiceProtocolClient>(
        Initialize(username, *m_relayServerConnection), Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Store(protocolClient->GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(protocolClient->GetSlots()));
      RegisterMarketDataRegistryMessages(Store(protocolClient->GetSlots()));
      authenticator(*protocolClient);
      FlushPendingRoutines();
      return protocolClient;
    }
  };
}

TEST_SUITE("MarketDataRelayServlet") {
  TEST_CASE_FIXTURE(Fixture, "validate_security") {
    auto relayClient = MakeMarketDataRelayClient("test_client");
    {
      auto snapshot = relayClient->SendRequest<QueryTimeAndSalesService>(
        MakeRealTimeQuery(TST_B));
      REQUIRE(snapshot.m_queryId != -1);
    }
    auto invalidSecurity = Security("TST_A", DefaultMarkets::OMGA(),
      DefaultCountries::CA());
    {
      auto snapshot = relayClient->SendRequest<QueryTimeAndSalesService>(
        MakeRealTimeQuery(invalidSecurity));
      REQUIRE(snapshot.m_queryId == -1);
    }
    m_marketDataEnvironment.GetFeedClient().Publish(SecurityTimeAndSale(
      TimeAndSale(time_from_string("2021-01-01 17:57:22"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
          "TSE", "CANA", "ITG"), TST_A));
    FlushPendingRoutines();
    m_marketDataEnvironment.GetFeedClient().Publish(SecurityTimeAndSale(
      TimeAndSale(time_from_string("2021-01-01 17:57:22"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
          "TSE", "CANA", "ITG"), TST_B));
    {
      auto timeAndSaleMessage = std::dynamic_pointer_cast<
        RecordMessage<TimeAndSaleMessage, TestServiceProtocolClient>>(
          relayClient->ReadMessage());
      REQUIRE(timeAndSaleMessage != nullptr);
      REQUIRE(
        timeAndSaleMessage->GetRecord().time_and_sale->GetIndex() == TST_B);
    }
    {
      auto snapshot = relayClient->SendRequest<QueryTimeAndSalesService>(
        MakeRealTimeQuery(TST_A));
      REQUIRE(snapshot.m_queryId != -1);
    }
    {
      auto snapshot = relayClient->SendRequest<QueryTimeAndSalesService>(
        MakeRealTimeQuery(invalidSecurity));
      REQUIRE(snapshot.m_queryId == -1);
    }
    m_marketDataEnvironment.GetFeedClient().Publish(SecurityTimeAndSale(
      TimeAndSale(time_from_string("2021-01-01 17:57:22"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
          "TSE", "CANA", "ITG"), TST_A));
    {
      auto timeAndSaleMessage = std::dynamic_pointer_cast<
        RecordMessage<TimeAndSaleMessage, TestServiceProtocolClient>>(
          relayClient->ReadMessage());
      REQUIRE(timeAndSaleMessage != nullptr);
      REQUIRE(
        timeAndSaleMessage->GetRecord().time_and_sale->GetIndex() == TST_A);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "primary_security_cache_invalidation") {
    auto relayClient = MakeMarketDataRelayClient("test_client");
    auto alternativeClient = MakeMarketDataRelayClient("test_client2");
    relayClient->SendRequest<QueryTimeAndSalesService>(
      MakeRealTimeQuery(TST_A));
    alternativeClient->SendRequest<QueryTimeAndSalesService>(
      MakeRealTimeQuery(TST_A));
    alternativeClient->SendRequest<QueryTimeAndSalesService>(
      MakeRealTimeQuery(TST_B));
    m_marketDataEnvironment.GetFeedClient().Publish(SecurityTimeAndSale(
      TimeAndSale(time_from_string("2021-01-01 17:57:22"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
          "TSE", "CANA", "ITG"), TST_A));
    relayClient->ReadMessage();
    alternativeClient->ReadMessage();
    auto updatedSecurity = Security("TST_A", DefaultMarkets::OMGA(),
      DefaultCountries::CA());
    m_marketDataEnvironment.GetFeedClient().Add(
      SecurityInfo(updatedSecurity, "TST_A Update", "", 200));
    {
      auto snapshot = relayClient->SendRequest<QueryTimeAndSalesService>(
        MakeRealTimeQuery(updatedSecurity));
      REQUIRE(snapshot.m_queryId != -1);
    }
    {
      auto snapshot = relayClient->SendRequest<QueryTimeAndSalesService>(
        MakeRealTimeQuery(TST_A));
      REQUIRE(snapshot.m_queryId == -1);
    }
    m_marketDataEnvironment.GetFeedClient().Publish(SecurityTimeAndSale(
      TimeAndSale(time_from_string("2021-01-01 17:58:22"), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
          "OMGA", "CANA", "ITG"), updatedSecurity));
    {
      auto timeAndSaleMessage = std::dynamic_pointer_cast<
        RecordMessage<TimeAndSaleMessage, TestServiceProtocolClient>>(
          relayClient->ReadMessage());
      REQUIRE(timeAndSaleMessage != nullptr);
      REQUIRE(timeAndSaleMessage->GetRecord().time_and_sale->GetIndex() ==
        updatedSecurity);
    }
  }
}
