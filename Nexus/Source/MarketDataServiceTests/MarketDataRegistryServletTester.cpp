#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
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
  const auto SECURITY_A =
    Security("ABX", DefaultMarkets::TSX(), DefaultCountries::CA());

  struct Fixture {
    using TestServletContainer =
      TestAuthenticatedServiceProtocolServletContainer<
        MetaMarketDataRegistryServlet<MarketDataRegistry*,
          LocalHistoricalDataStore, AdministrationClientBox>,
        NativePointerPolicy>;

    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    AdministrationServiceTestEnvironment m_administrationEnvironment;
    MarketDataRegistry m_registry;
    boost::optional<TestServletContainer::Servlet::Servlet> m_registryServlet;
    boost::optional<TestServletContainer> m_container;
    boost::optional<TestServiceProtocolClient> m_protocolClient;

    Fixture()
        : m_administrationEnvironment(
            m_serviceLocatorEnvironment.MakeClient(), BuildEntitlements()) {
      auto servletServiceLocatorClient =
        m_serviceLocatorEnvironment.MakeClient();
      m_registryServlet.emplace(m_administrationEnvironment.MakeClient(
        servletServiceLocatorClient), &m_registry, Initialize());
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_container.emplace(Initialize(std::move(servletServiceLocatorClient),
        &*m_registryServlet), serverConnection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_protocolClient.emplace(Initialize("test", *serverConnection),
        Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Store(m_protocolClient->GetSlots().GetRegistry()));
      RegisterMarketDataRegistryServices(Store(m_protocolClient->GetSlots()));
      RegisterMarketDataRegistryMessages(Store(m_protocolClient->GetSlots()));
      auto clientServiceLocatorClient =
        m_serviceLocatorEnvironment.MakeClient("client", "");
      auto authenticator = SessionAuthenticator(clientServiceLocatorClient);
      authenticator(*m_protocolClient);
    }

    EntitlementDatabase BuildEntitlements() {
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
      auto entitlements = EntitlementDatabase();
      entitlements.Add(nyseEntitlement);
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
      entitlements.Add(tsxEntitlements);
      m_serviceLocatorEnvironment.GetRoot().Associate(clientEntry,
        nyseEntitlementGroup);
      m_serviceLocatorEnvironment.GetRoot().Associate(clientEntry,
        tsxEntitlementGroup);
      return entitlements;
    }
  };
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE_FIXTURE(Fixture, "market_and_source_entitlement") {
    auto query = SecurityMarketDataQuery();
    query.SetIndex(SECURITY_A);
    query.SetRange(Range::RealTime());
    auto snapshot = m_protocolClient->SendRequest<QueryBookQuotesService>(
      query);
    auto bookQuote = SecurityBookQuote(
      BookQuote("CHIC", false, DefaultMarkets::CHIC(),
        Quote(Money::ONE, 100, Side::BID), second_clock::universal_time()),
      SECURITY_A);
    m_registryServlet->UpdateBookQuote(bookQuote, 1);
  }

  TEST_CASE_FIXTURE(Fixture, "query_security_info") {
    auto query = SecurityInfoQuery();
    query.SetIndex(Region::Global());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto info = SecurityInfo(SECURITY_A, "ABX", "Mining", 100);
    m_registryServlet->Add(info);
    auto result = m_protocolClient->SendRequest<QuerySecurityInfoService>(
      query);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == info);
  }
}
