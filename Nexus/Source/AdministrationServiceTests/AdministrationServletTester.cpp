#include <boost/functional/factory.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;

namespace {
  struct Fixture {
    using ServletContainer = TestServiceProtocolServletContainer<
      MetaAdministrationServlet<ServiceLocatorClientBox,
        LocalAdministrationDataStore*>>;

    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    LocalAdministrationDataStore m_dataStore;
    std::shared_ptr<TestServerConnection> m_serverConnection;
    ServletContainer m_container;
    TestServiceProtocolClient m_protocolClient;

    Fixture()
      : m_serverConnection(std::make_shared<TestServerConnection>()),
        m_container(Initialize(&m_serviceLocatorEnvironment.GetRoot(),
          EntitlementDatabase(), &m_dataStore), m_serverConnection,
          factory<std::unique_ptr<TriggerTimer>>()),
        m_protocolClient(Initialize("test", *m_serverConnection),
          Initialize()) {
      RegisterAdministrationServices(Store(m_protocolClient.GetSlots()));
      RegisterAdministrationMessages(Store(m_protocolClient.GetSlots()));
    }

    auto GetAdministratorsDirectory() {
      return m_serviceLocatorEnvironment.GetRoot().LoadDirectoryEntry(
        DirectoryEntry::GetStarDirectory(), "administrators");
    }

    auto GetServicesDirectory() {
      return m_serviceLocatorEnvironment.GetRoot().LoadDirectoryEntry(
        DirectoryEntry::GetStarDirectory(), "services");
    }

    auto GetTradingGroupsDirectory() {
      return m_serviceLocatorEnvironment.GetRoot().LoadDirectoryEntry(
        DirectoryEntry::GetStarDirectory(), "trading_groups");
    }

    auto MakeAccount(const std::string& name, const DirectoryEntry& parent) {
      return m_serviceLocatorEnvironment.GetRoot().MakeAccount(name, "1234",
        parent);
    }

    auto MakeTradingGroup(const std::string& name) {
      auto& rootServiceLocatorClient = m_serviceLocatorEnvironment.GetRoot();
      auto tradingGroups = rootServiceLocatorClient.MakeDirectory(name,
        GetTradingGroupsDirectory());
      auto managers = rootServiceLocatorClient.MakeDirectory("managers",
        tradingGroups);
      auto traders = rootServiceLocatorClient.MakeDirectory("traders",
        tradingGroups);
      return tradingGroups;
    }
  };
}

TEST_SUITE("AdministrationServlet") {
#if 0
  TEST_CASE_FIXTURE(Fixture, "load_account_identity") {
    auto account = MakeAccount("test_account",
      DirectoryEntry::GetStarDirectory());
    auto accountIdentity = AccountIdentity();
    accountIdentity.m_firstName = "test_first_name";
    accountIdentity.m_lastName = "test_last_name";
    m_dataStore.Store(account, accountIdentity);
    auto accountIdentityResult = m_protocolClient.SendRequest<
      LoadAccountIdentityService>(account);
    REQUIRE(accountIdentityResult.m_firstName == accountIdentity.m_firstName);
    REQUIRE(accountIdentityResult.m_lastName == accountIdentity.m_lastName);
  }

  TEST_CASE_FIXTURE(Fixture, "invalid_load_account_identity") {
    auto invalidAccount = DirectoryEntry::MakeAccount(123, "dummy");
    REQUIRE_THROWS_AS(
      m_protocolClient.SendRequest<LoadAccountIdentityService>(invalidAccount),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "initial_set_account_identity") {
    auto account = MakeAccount("test_account",
      DirectoryEntry::GetStarDirectory());
    auto sendAccountIdentity = AccountIdentity();
    sendAccountIdentity.m_firstName = "send";
    sendAccountIdentity.m_lastName = "account";
    m_protocolClient.SendRequest<StoreAccountIdentityService>(account,
      sendAccountIdentity);
    auto receiveAccountIdentity = AccountIdentity();
    m_dataStore.WithTransaction(
      [&] {
        receiveAccountIdentity = m_dataStore.LoadIdentity(account);
      });
    REQUIRE(receiveAccountIdentity.m_firstName ==
      sendAccountIdentity.m_firstName);
    REQUIRE(receiveAccountIdentity.m_lastName ==
      sendAccountIdentity.m_lastName);
  }

  TEST_CASE_FIXTURE(Fixture, "existing_set_account_identity") {
    auto account = MakeAccount("test_account",
      DirectoryEntry::GetStarDirectory());
    auto accountIdentity = AccountIdentity();
    accountIdentity.m_firstName = "test_first_name";
    accountIdentity.m_lastName = "test_last_name";
    m_dataStore.Store(account, accountIdentity);
    auto sendAccountIdentity = AccountIdentity();
    sendAccountIdentity.m_firstName = "send";
    sendAccountIdentity.m_lastName = "account";
    m_protocolClient.SendRequest<StoreAccountIdentityService>(account,
      sendAccountIdentity);
    auto receiveAccountIdentity = AccountIdentity();
    m_dataStore.WithTransaction(
      [&] {
        receiveAccountIdentity = m_dataStore.LoadIdentity(account);
      });
    REQUIRE(receiveAccountIdentity.m_firstName ==
      sendAccountIdentity.m_firstName);
    REQUIRE(receiveAccountIdentity.m_lastName ==
      sendAccountIdentity.m_lastName);
  }

  TEST_CASE_FIXTURE(Fixture, "invalid_set_account_identity") {
    auto account = DirectoryEntry::MakeAccount(1234, "dummy");
    auto sendAccountIdentity = AccountIdentity();
    sendAccountIdentity.m_firstName = "send";
    sendAccountIdentity.m_lastName = "account";
    REQUIRE_THROWS_AS(m_protocolClient.SendRequest<
      StoreAccountIdentityService>(account, sendAccountIdentity),
      ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "load_administrators") {
    auto administratorA = MakeAccount("adminA", GetAdministratorsDirectory());
    auto administratorB = MakeAccount("adminB", GetAdministratorsDirectory());
    auto administratorsResult =
      m_protocolClient.SendRequest<LoadAdministratorsService>();
    REQUIRE(administratorsResult.size() == 2);
  }

  TEST_CASE_FIXTURE(Fixture, "load_services") {
    auto serviceA = MakeAccount("serviceA", GetServicesDirectory());
    auto serviceB = MakeAccount("serviceB", GetServicesDirectory());
    auto servicesResult = m_protocolClient.SendRequest<LoadServicesService>();
    REQUIRE(servicesResult.size() == 2);
    auto services = std::unordered_set<DirectoryEntry>();
    services.insert(serviceA);
    services.insert(serviceB);
    services.erase(servicesResult[0]);
    services.erase(servicesResult[1]);
    REQUIRE(services.empty());
  }

  TEST_CASE_FIXTURE(Fixture, "load_managed_trading_groups") {
    auto tradingGroupA = MakeTradingGroup("test_group_a");
    auto tradingGroupB = MakeTradingGroup("test_group_b");
    auto managers =
      m_serviceLocatorEnvironment.GetRoot().LoadDirectoryEntry(tradingGroupA,
      "managers");
    auto traders =
      m_serviceLocatorEnvironment.GetRoot().LoadDirectoryEntry(tradingGroupA,
      "traders");
    auto manager = MakeAccount("test_manager", managers);
    auto trader = MakeAccount("test_trader", traders);
    auto managedTradingGroupsResult =
      m_protocolClient.SendRequest<LoadManagedTradingGroupsService>(manager);
    REQUIRE(managedTradingGroupsResult.size() == 1);
    REQUIRE(managedTradingGroupsResult.front() == tradingGroupA);
    managedTradingGroupsResult =
      m_protocolClient.SendRequest<LoadManagedTradingGroupsService>(trader);
    REQUIRE(managedTradingGroupsResult.empty());
  }
#endif
}
