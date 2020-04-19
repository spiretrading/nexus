#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;

namespace {
  auto GetTsxTestSecurity() {
    return Security("ABX", DefaultMarkets::TSX(), DefaultCountries::CA());
  }

  struct Fixture {
    using TestServletContainer =
      TestAuthenticatedServiceProtocolServletContainer<
      MetaMarketDataRegistryServlet<MarketDataRegistry*,
      LocalHistoricalDataStore, std::unique_ptr<VirtualAdministrationClient>>,
      NativePointerPolicy>;

    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    boost::optional<AdministrationServiceTestEnvironment>
      m_administrationEnvironment;
    EntitlementDatabase m_entitlements;
    MarketDataRegistry m_registry;
    boost::optional<TestServletContainer::Servlet::Servlet> m_registryServlet;
    boost::optional<TestServletContainer> m_container;
    boost::optional<Beam::Services::Tests::TestServiceProtocolClient>
      m_clientProtocol;

    Fixture() {
      m_serviceLocatorEnvironment.Open();
      auto administrationServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      administrationServiceLocatorClient->SetCredentials("root", "");
      auto servletAccount = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "servlet", "", DirectoryEntry::GetStarDirectory());
      auto clientEntry = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "client", "", DirectoryEntry::GetStarDirectory());
      auto entitlementsDirectory =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory("entitlements",
        DirectoryEntry::GetStarDirectory());
      auto nyseEntitlementGroup =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory(
        "NYSE", entitlementsDirectory);
      auto servletPermissions = Permissions();
      servletPermissions.Set(Permission::READ);
      servletPermissions.Set(Permission::MOVE);
      servletPermissions.Set(Permission::ADMINISTRATE);
      m_serviceLocatorEnvironment.GetRoot().StorePermissions(servletAccount,
        entitlementsDirectory, servletPermissions);
      auto tsxEntitlementGroup =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory("TSX",
        entitlementsDirectory);
      auto nyseEntitlement = EntitlementDatabase::Entry();
      nyseEntitlement.m_name = "NYSE";
      nyseEntitlement.m_groupEntry = nyseEntitlementGroup;
      nyseEntitlement.m_applicability[DefaultMarkets::NYSE()].Set(
        MarketDataType::BBO_QUOTE);
      nyseEntitlement.m_applicability[DefaultMarkets::NYSE()].Set(
        MarketDataType::ORDER_IMBALANCE);
      m_entitlements.Add(nyseEntitlement);
      auto tsxEntitlements = EntitlementDatabase::Entry();
      tsxEntitlements.m_name = "TSX";
      tsxEntitlements.m_groupEntry = tsxEntitlementGroup;
      auto tsxKey = EntitlementKey(DefaultMarkets::TSX(),
        DefaultMarkets::TSX());
      tsxEntitlements.m_applicability[tsxKey].Set(MarketDataType::BOOK_QUOTE);
      auto chicTsxKey = EntitlementKey(DefaultMarkets::TSX(),
        DefaultMarkets::CHIC());
      tsxEntitlements.m_applicability[chicTsxKey].Set(
        MarketDataType::BOOK_QUOTE);
      m_entitlements.Add(tsxEntitlements);
      m_serviceLocatorEnvironment.GetRoot().Associate(clientEntry,
        nyseEntitlementGroup);
      m_serviceLocatorEnvironment.GetRoot().Associate(clientEntry,
        tsxEntitlementGroup);
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_clientProtocol.emplace(Initialize("test", Ref(*serverConnection)),
        Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Store(m_clientProtocol->GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(m_clientProtocol->GetSlots()));
      RegisterMarketDataRegistryMessages(Store(m_clientProtocol->GetSlots()));
      auto servletServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      servletServiceLocatorClient->SetCredentials("servlet", "");
      servletServiceLocatorClient->Open();
      m_administrationEnvironment.emplace(
        std::move(administrationServiceLocatorClient));
      m_administrationEnvironment->SetEntitlements(m_entitlements);
      m_administrationEnvironment->Open();
      auto marketDataAdministrationClient =
        m_administrationEnvironment->BuildClient(
        Ref(*servletServiceLocatorClient));
      m_registryServlet.emplace(std::move(marketDataAdministrationClient),
        &m_registry, Initialize());
      m_container.emplace(Initialize(std::move(servletServiceLocatorClient),
        &*m_registryServlet), serverConnection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
      auto clientServiceLocatorClient =
        m_serviceLocatorEnvironment.BuildClient();
      clientServiceLocatorClient->SetCredentials("client", "");
      clientServiceLocatorClient->Open();
      m_clientProtocol->Open();
      auto authenticator = SessionAuthenticator(
        Ref(*clientServiceLocatorClient));
      authenticator(*m_clientProtocol);
    }
  };
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE_FIXTURE(Fixture, "market_and_source_entitlement") {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(GetTsxTestSecurity());
    query.SetRange(Range::RealTime());
    auto snapshot = m_clientProtocol->SendRequest<QueryBookQuotesService>(
      query);
    auto bookQuote = SecurityBookQuote(
      BookQuote("CHIC", false, DefaultMarkets::CHIC(),
      Quote(Money::ONE, 100, Side::BID), second_clock::universal_time()),
      GetTsxTestSecurity());
    m_registryServlet->UpdateBookQuote(bookQuote, 1);
  }
}
