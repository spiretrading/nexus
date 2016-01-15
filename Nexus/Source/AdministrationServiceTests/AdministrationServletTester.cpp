#include "Nexus/AdministrationServiceTests/AdministrationServletTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace std;

void AdministrationServletTester::setUp() {
  m_serviceLocatorInstance.Initialize();
  m_serviceLocatorInstance->Open();
  m_dataStore = std::make_shared<LocalAdministrationDataStore>();
  m_serverConnection = std::make_shared<ServerConnection>();
  m_clientProtocol.Initialize(Initialize(string("test"),
    Ref(*m_serverConnection)), Initialize());
  RegisterAdministrationServices(Store(m_clientProtocol->GetSlots()));
  RegisterAdministrationMessages(Store(m_clientProtocol->GetSlots()));
  m_container.Initialize(Initialize(&m_serviceLocatorInstance->GetRoot(),
    EntitlementDatabase(), m_dataStore), m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void AdministrationServletTester::tearDown() {
  m_clientProtocol.Reset();
  m_container.Reset();
  m_dataStore.reset();
  m_serviceLocatorInstance.Reset();
}

void AdministrationServletTester::TestLoadAccountIdentity() {
  auto account = MakeAccount("test_account",
    DirectoryEntry::GetStarDirectory());
  AccountIdentity accountIdentity;
  accountIdentity.m_firstName = "test_first_name";
  accountIdentity.m_lastName = "test_last_name";
  m_dataStore->Store(account, accountIdentity);
  auto accountIdentityResult = m_clientProtocol->SendRequest<
    LoadAccountIdentityService>(account);
  CPPUNIT_ASSERT(accountIdentityResult.m_firstName ==
    accountIdentity.m_firstName);
  CPPUNIT_ASSERT(accountIdentityResult.m_lastName ==
    accountIdentity.m_lastName);
}

void AdministrationServletTester::TestInvalidLoadAccountIdentity() {
  DirectoryEntry invalidAccount(DirectoryEntry::Type::ACCOUNT, 123, "dummy");
  CPPUNIT_ASSERT_THROW(
    m_clientProtocol->SendRequest<LoadAccountIdentityService>(invalidAccount),
    ServiceRequestException);
}

void AdministrationServletTester::TestInitialSetAccountIdentity() {
  auto account = MakeAccount("test_account",
    DirectoryEntry::GetStarDirectory());
  AccountIdentity sendAccountIdentity;
  sendAccountIdentity.m_firstName = "send";
  sendAccountIdentity.m_lastName = "account";
  m_clientProtocol->SendRequest<StoreAccountIdentityService>(account,
    sendAccountIdentity);
  AccountIdentity receiveAccountIdentity;
  m_dataStore->WithTransaction(
    [&] {
      receiveAccountIdentity = m_dataStore->LoadIdentity(account);
    });
  CPPUNIT_ASSERT(receiveAccountIdentity.m_firstName ==
    sendAccountIdentity.m_firstName);
  CPPUNIT_ASSERT(receiveAccountIdentity.m_lastName ==
    sendAccountIdentity.m_lastName);
}

void AdministrationServletTester::TestExistingSetAccountIdentity() {
  auto account = MakeAccount("test_account",
    DirectoryEntry::GetStarDirectory());
  AccountIdentity accountIdentity;
  accountIdentity.m_firstName = "test_first_name";
  accountIdentity.m_lastName = "test_last_name";
  m_dataStore->Store(account, accountIdentity);
  AccountIdentity sendAccountIdentity;
  sendAccountIdentity.m_firstName = "send";
  sendAccountIdentity.m_lastName = "account";
  m_clientProtocol->SendRequest<StoreAccountIdentityService>(account,
    sendAccountIdentity);
  AccountIdentity receiveAccountIdentity;
  m_dataStore->WithTransaction(
    [&] {
      receiveAccountIdentity = m_dataStore->LoadIdentity(account);
    });
  CPPUNIT_ASSERT(receiveAccountIdentity.m_firstName ==
    sendAccountIdentity.m_firstName);
  CPPUNIT_ASSERT(receiveAccountIdentity.m_lastName ==
    sendAccountIdentity.m_lastName);
}

void AdministrationServletTester::TestInvalidSetAccountIdentity() {
  DirectoryEntry account(DirectoryEntry::Type::ACCOUNT, 1234, "dummy");
  AccountIdentity sendAccountIdentity;
  sendAccountIdentity.m_firstName = "send";
  sendAccountIdentity.m_lastName = "account";
  CPPUNIT_ASSERT_THROW(m_clientProtocol->SendRequest<
    StoreAccountIdentityService>(account, sendAccountIdentity),
    ServiceRequestException);
}

void AdministrationServletTester::TestLoadAdministrators() {
  auto administratorA = MakeAccount("adminA", GetAdministratorsDirectory());
  auto administratorB = MakeAccount("adminB", GetAdministratorsDirectory());
  auto administratorsResult =
    m_clientProtocol->SendRequest<LoadAdministratorsService>(0);
  CPPUNIT_ASSERT(administratorsResult.size() == 2);
}

void AdministrationServletTester::TestLoadServices() {
  auto serviceA = MakeAccount("serviceA", GetServicesDirectory());
  auto serviceB = MakeAccount("serviceB", GetServicesDirectory());
  auto servicesResult = m_clientProtocol->SendRequest<LoadServicesService>(0);
  CPPUNIT_ASSERT(servicesResult.size() == 2);
  unordered_set<DirectoryEntry> services;
  services.insert(serviceA);
  services.insert(serviceB);
  services.erase(servicesResult[0]);
  services.erase(servicesResult[1]);
  CPPUNIT_ASSERT(services.empty());
}

void AdministrationServletTester::TestLoadManagedTradingGroups() {
  auto tradingGroupA = MakeTradingGroup("test_group_a");
  auto tradingGroupB = MakeTradingGroup("test_group_b");
  auto managers =
    m_serviceLocatorInstance->GetRoot().LoadDirectoryEntry(tradingGroupA,
    "managers");
  auto traders =
    m_serviceLocatorInstance->GetRoot().LoadDirectoryEntry(tradingGroupA,
    "traders");
  auto manager = MakeAccount("test_manager", managers);
  auto trader = MakeAccount("test_trader", traders);
  auto managedTradingGroupsResult =
    m_clientProtocol->SendRequest<LoadManagedTradingGroupsService>(manager);
  CPPUNIT_ASSERT(managedTradingGroupsResult.size() == 1);
  CPPUNIT_ASSERT(managedTradingGroupsResult.front() == tradingGroupA);
  managedTradingGroupsResult =
    m_clientProtocol->SendRequest<LoadManagedTradingGroupsService>(trader);
  CPPUNIT_ASSERT(managedTradingGroupsResult.empty());
}

DirectoryEntry AdministrationServletTester::GetAdministratorsDirectory() {
  return m_serviceLocatorInstance->GetRoot().LoadDirectoryEntry(
    DirectoryEntry::GetStarDirectory(), "administrators");
}

DirectoryEntry AdministrationServletTester::GetServicesDirectory() {
  return m_serviceLocatorInstance->GetRoot().LoadDirectoryEntry(
    DirectoryEntry::GetStarDirectory(), "services");
}

DirectoryEntry AdministrationServletTester::GetTradingGroupsDirectory() {
  return m_serviceLocatorInstance->GetRoot().LoadDirectoryEntry(
    DirectoryEntry::GetStarDirectory(), "trading_groups");
}

DirectoryEntry AdministrationServletTester::MakeAccount(const string& name,
    const DirectoryEntry& parent) {
  return m_serviceLocatorInstance->GetRoot().MakeAccount(name, "1234", parent);
}

DirectoryEntry AdministrationServletTester::MakeTradingGroup(
    const string& name) {
  auto& rootServiceLocatorClient = m_serviceLocatorInstance->GetRoot();
  auto tradingGroups = rootServiceLocatorClient.MakeDirectory(name,
    GetTradingGroupsDirectory());
  auto managers = rootServiceLocatorClient.MakeDirectory("managers",
    tradingGroups);
  auto traders = rootServiceLocatorClient.MakeDirectory("traders",
    tradingGroups);
  return tradingGroups;
}
