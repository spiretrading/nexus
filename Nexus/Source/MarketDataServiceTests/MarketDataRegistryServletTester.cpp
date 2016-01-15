#include "Nexus/MarketDataServiceTests/MarketDataRegistryServletTester.hpp"
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Serialization;
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
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  auto servletAccount = m_serviceLocatorInstance->GetRoot().MakeAccount(
    "servlet", "", DirectoryEntry::GetStarDirectory());
  auto clientEntry = m_serviceLocatorInstance->GetRoot().MakeAccount("client",
    "", DirectoryEntry::GetStarDirectory());
  auto entitlementsDirectory =
    m_serviceLocatorInstance->GetRoot().MakeDirectory("entitlements",
    DirectoryEntry::GetStarDirectory());
  auto nyseEntitlementGroup = m_serviceLocatorInstance->GetRoot().MakeDirectory(
    "NYSE", entitlementsDirectory);
  Permissions servletPermissions;
  servletPermissions.Set(Permission::READ);
  servletPermissions.Set(Permission::MOVE);
  servletPermissions.Set(Permission::ADMINISTRATE);
  m_serviceLocatorInstance->GetRoot().StorePermissions(servletAccount,
    entitlementsDirectory, servletPermissions);
  auto tsxEntitlementGroup = m_serviceLocatorInstance->GetRoot().MakeDirectory(
    "TSX", entitlementsDirectory);
  m_entitlements.Initialize();
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
  m_serviceLocatorInstance->GetRoot().Associate(clientEntry,
    nyseEntitlementGroup);
  m_serviceLocatorInstance->GetRoot().Associate(clientEntry,
    tsxEntitlementGroup);
  m_serverConnection.Initialize();
  m_clientProtocol.Initialize(Initialize(string("test"),
    Ref(*m_serverConnection)), Initialize());
  Nexus::Queries::RegisterQueryTypes(
    Store(m_clientProtocol->GetSlots().GetRegistry()));
  RegisterMarketDataRegistryServices(Store(m_clientProtocol->GetSlots()));
  RegisterMarketDataRegistryMessages(Store(m_clientProtocol->GetSlots()));
  m_servletServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  m_servletServiceLocatorClient->SetCredentials("servlet", "");
  m_servletServiceLocatorClient->Open();
  m_registry.Initialize();
  m_registryServlet.Initialize(*m_entitlements,
    Ref(*m_servletServiceLocatorClient), &*m_registry, Initialize());
  m_servlet.Initialize(&*m_servletServiceLocatorClient, &*m_registryServlet);
  m_container.Initialize(&*m_servlet, &*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientServiceLocatorClient = m_serviceLocatorInstance->BuildClient();
  m_clientServiceLocatorClient->SetCredentials("client", "");
  m_clientServiceLocatorClient->Open();
  m_clientProtocol->Open();
  SessionAuthenticator<TestServiceLocatorClient> authenticator(
    Ref(*m_clientServiceLocatorClient));
  authenticator(*m_clientProtocol);
}

void MarketDataRegistryServletTester::tearDown() {
  m_clientProtocol.Reset();
  m_clientServiceLocatorClient.reset();
  m_container.Reset();
  m_servlet.Reset();
  m_registryServlet.Reset();
  m_registry.Reset();
  m_entitlements.Reset();
  m_servletServiceLocatorClient.reset();
  m_serviceLocatorInstance->Close();
}

void MarketDataRegistryServletTester::TestMarketAndSourceEntitlement() {
  SecurityMarketDataQuery query;
  query.SetIndex(GetTsxTestSecurity());
  query.SetRange(Range::RealTime());
  auto snapshot = m_clientProtocol->SendRequest<QueryBookQuotesService>(query);
  SecurityBookQuote bookQuote(BookQuote("CHIC", false, DefaultMarkets::CHIC(),
    Quote(Money::ONE, 100, Side::BID), second_clock::universal_time()),
    GetTsxTestSecurity());
  m_registryServlet->UpdateBookQuote(bookQuote, 1);
}
