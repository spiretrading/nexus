#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRelayServlet.hpp"

using namespace Beam;
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

namespace {
  using TestServletContainer =
    TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRelayServlet<MarketDataClientBox, AdministrationClientBox>>;
  using TestMarketDataClient =
    MarketDataClient<TestServiceProtocolClientBuilder>;

  struct Fixture {
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    AdministrationServiceTestEnvironment m_administrationEnvironment;
    std::shared_ptr<TestServerConnection> m_marketDataServerConnection;
    TestServiceProtocolServer m_marketDataServer;
    std::shared_ptr<TestServerConnection> m_relayServerConnection;
    optional<TestServletContainer> m_container;

    Fixture()
        : m_administrationEnvironment(MakeAdministrationServiceTestEnvironment(
            m_serviceLocatorEnvironment)),
          m_marketDataServerConnection(
            std::make_shared<TestServerConnection>()),
          m_marketDataServer(m_marketDataServerConnection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()),
          m_relayServerConnection(std::make_shared<TestServerConnection>()) {
      Nexus::Queries::RegisterQueryTypes(
        Store(m_marketDataServer.GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(m_marketDataServer.GetSlots()));
      RegisterMarketDataRegistryMessages(Store(m_marketDataServer.GetSlots()));
      auto marketDataClientFactory = [&] {
        return std::make_unique<MarketDataClientBox>(MakeMarketDataClient());
      };
      m_container.emplace(Initialize(m_serviceLocatorEnvironment.GetRoot(),
        Initialize(seconds(100), marketDataClientFactory, 1, 1,
          m_administrationEnvironment.GetClient())), m_relayServerConnection,
        factory<std::unique_ptr<TriggerTimer>>());
    }

    MarketDataClientBox MakeMarketDataClient() {
      return MarketDataClientBox(std::in_place_type<TestMarketDataClient>,
        TestServiceProtocolClientBuilder(std::bind(
          factory<std::unique_ptr<TestServiceProtocolClientBuilder::Channel>>(),
          "test", std::ref(*m_marketDataServerConnection)),
          factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>()));
    }

    std::unique_ptr<TestServiceProtocolClient> MakeMarketDataRelayClient(
        const std::string& username) {
      m_serviceLocatorEnvironment.GetRoot().MakeAccount(username, "1234",
        DirectoryEntry::GetStarDirectory());
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
      protocolClient->SpawnMessageHandler();
      return protocolClient;
    }
  };
}

TEST_SUITE("MarketDataRelayServlet") {
  TEST_CASE_FIXTURE(Fixture, "validate_security") {
    auto relayClient = MakeMarketDataRelayClient("test_client");
  }
}
