#include "Nexus/MarketDataServiceTests/MarketDataRegistryServletTester.hpp"
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
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
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace std;

namespace {
  Security GetTsxTestSecurity() {
    Security security{"ABX", DefaultMarkets::TSX(), DefaultCountries::CA()};
    return security;
  }
}

void MarketDataRegistryServletTester::setUp() {
  m_serviceLocatorEnvironment.emplace();
  m_serviceLocatorEnvironment->Open();
  auto administrationServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  administrationServiceLocatorClient->SetCredentials("root", "");
  auto servletAccount = m_serviceLocatorEnvironment->GetRoot().MakeAccount(
    "servlet", "", DirectoryEntry::GetStarDirectory());
  auto clientEntry = m_serviceLocatorEnvironment->GetRoot().MakeAccount(
    "client", "", DirectoryEntry::GetStarDirectory());
  auto entitlementsDirectory =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory("entitlements",
    DirectoryEntry::GetStarDirectory());
  auto nyseEntitlementGroup =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory(
    "NYSE", entitlementsDirectory);
  Permissions servletPermissions;
  servletPermissions.Set(Permission::READ);
  servletPermissions.Set(Permission::MOVE);
  servletPermissions.Set(Permission::ADMINISTRATE);
  m_serviceLocatorEnvironment->GetRoot().StorePermissions(servletAccount,
    entitlementsDirectory, servletPermissions);
  auto tsxEntitlementGroup =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory(
    "TSX", entitlementsDirectory);
  m_entitlements.emplace();
  EntitlementDatabase::Entry nyseEntitlement;
  nyseEntitlement.m_name = "NYSE";
  nyseEntitlement.m_groupEntry = nyseEntitlementGroup;
  nyseEntitlement.m_applicability[DefaultMarkets::NYSE()].Set(
    MarketDataType::BBO_QUOTE);
  nyseEntitlement.m_applicability[DefaultMarkets::NYSE()].Set(
    MarketDataType::ORDER_IMBALANCE);
  m_entitlements->Add(nyseEntitlement);
  EntitlementDatabase::Entry tsxEntitlements;
  tsxEntitlements.m_name = "TSX";
  tsxEntitlements.m_groupEntry = tsxEntitlementGroup;
  EntitlementKey tsxKey{DefaultMarkets::TSX(), DefaultMarkets::TSX()};
  tsxEntitlements.m_applicability[tsxKey].Set(MarketDataType::BOOK_QUOTE);
  EntitlementKey chicTsxKey{DefaultMarkets::TSX(), DefaultMarkets::CHIC()};
  tsxEntitlements.m_applicability[chicTsxKey].Set(MarketDataType::BOOK_QUOTE);
  m_entitlements->Add(tsxEntitlements);
  m_serviceLocatorEnvironment->GetRoot().Associate(clientEntry,
    nyseEntitlementGroup);
  m_serviceLocatorEnvironment->GetRoot().Associate(clientEntry,
    tsxEntitlementGroup);
  auto serverConnection = std::make_shared<TestServerConnection>();
  m_clientProtocol.emplace(Initialize("test", Ref(*serverConnection)),
    Initialize());
  Nexus::Queries::RegisterQueryTypes(
    Store(m_clientProtocol->GetSlots().GetRegistry()));
  RegisterMarketDataRegistryServices(Store(m_clientProtocol->GetSlots()));
  RegisterMarketDataRegistryMessages(Store(m_clientProtocol->GetSlots()));
  auto servletServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  servletServiceLocatorClient->SetCredentials("servlet", "");
  servletServiceLocatorClient->Open();
  m_administrationEnvironment.emplace(
    std::move(administrationServiceLocatorClient));
  m_administrationEnvironment->SetEntitlements(*m_entitlements);
  m_administrationEnvironment->Open();
  auto marketDataAdministrationClient =
    m_administrationEnvironment->BuildClient(Ref(*servletServiceLocatorClient));
  m_registry.emplace();
  m_registryServlet.emplace(std::move(marketDataAdministrationClient),
    &*m_registry, Initialize());
  m_container.emplace(Initialize(std::move(servletServiceLocatorClient),
    &*m_registryServlet), serverConnection,
    factory<std::unique_ptr<TriggerTimer>>());
  m_container->Open();
  auto clientServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  clientServiceLocatorClient->SetCredentials("client", "");
  clientServiceLocatorClient->Open();
  m_clientProtocol->Open();
  SessionAuthenticator<VirtualServiceLocatorClient> authenticator{
    Ref(*clientServiceLocatorClient)};
  authenticator(*m_clientProtocol);
}

void MarketDataRegistryServletTester::tearDown() {
  m_clientProtocol.reset();
  m_container.reset();
  m_registryServlet.reset();
  m_registry.reset();
  m_entitlements.reset();
  m_serviceLocatorEnvironment->Close();
}

void MarketDataRegistryServletTester::TestMarketAndSourceEntitlement() {
  SecurityMarketDataQuery query;
  query.SetIndex(GetTsxTestSecurity());
  query.SetRange(Range::RealTime());
  auto snapshot = m_clientProtocol->SendRequest<QueryBookQuotesService>(query);
  SecurityBookQuote bookQuote{BookQuote{"CHIC", false, DefaultMarkets::CHIC(),
    Quote{Money::ONE, 100, Side::BID}, second_clock::universal_time()},
    GetTsxTestSecurity()};
  m_registryServlet->UpdateBookQuote(bookQuote, 1);
}
