#include <boost/functional/factory.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaAdministrationServlet<ServiceLocatorClientBox,
        LocalAdministrationDataStore*, FixedTimeClient*>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    optional<ServiceLocatorClientBox> m_servlet_service_locator_client;
    LocalAdministrationDataStore m_data_store;
    std::shared_ptr<TestServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    DirectoryEntry m_admin_account;
    std::unique_ptr<TestServiceProtocolClient> m_admin_client;
    DirectoryEntry m_trader_account;
    std::unique_ptr<TestServiceProtocolClient> m_trader_client;

    Fixture()
      : m_time_client(time_from_string("2024-07-04 12:00:00")),
        m_server_connection(std::make_shared<TestServerConnection>()) {
      auto servlet_account = make_account(
        "administration_service", DirectoryEntry::GetStarDirectory());
      m_servlet_service_locator_client =
        m_service_locator_environment.MakeClient(servlet_account.m_name, "");
      m_container.emplace(Initialize(*m_servlet_service_locator_client,
        Initialize(&m_service_locator_environment.GetRoot(),
          EntitlementDatabase(), &m_data_store, &m_time_client)),
          m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      auto admin_group =
        m_service_locator_environment.GetRoot().LoadDirectoryEntry(
          DirectoryEntry::GetStarDirectory(), "administrators");
      make_account("admin", admin_group);
      make_account("trader", DirectoryEntry::GetStarDirectory());
      std::tie(m_admin_account, m_admin_client) = make_client("admin");
      std::tie(m_trader_account, m_trader_client) = make_client("trader");
    }

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.GetRoot().MakeAccount(
        name, "", parent);
    }

    auto make_trading_group(const std::string& name) {
      auto& service_locator = m_service_locator_environment.GetRoot();
      auto trading_groups_root = service_locator.LoadDirectoryEntry(
        DirectoryEntry::GetStarDirectory(), "trading_groups");
      auto trading_group = service_locator.MakeDirectory(
        "test_trading_group", trading_groups_root);
      auto managers_group =
        service_locator.MakeDirectory("managers", trading_group);
      auto traders_group =
        service_locator.MakeDirectory("traders", trading_group);
      return TradingGroup(trading_group, managers_group, {}, traders_group, {});
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.MakeClient(name, "");
      auto authenticator = SessionAuthenticator(service_locator_client);
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        Initialize(name, *m_server_connection), Initialize());
      RegisterAdministrationServices(Store(protocol_client->GetSlots()));
      RegisterAdministrationMessages(Store(protocol_client->GetSlots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.GetAccount(), std::move(protocol_client));
    }
  };
}

TEST_SUITE("AdministrationServlet") {
  TEST_CASE("directories_initialized") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    REQUIRE_NOTHROW(service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "administrators"));
    REQUIRE_NOTHROW(service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services"));
    REQUIRE_NOTHROW(service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "trading_groups"));
  }

  TEST_CASE("load_accounts_by_roles") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto admin_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "administrators");
    auto services_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services");
    auto admin_account = fixture.make_account("b", admin_group);
    auto service_account = fixture.make_account("c", services_group);
    SUBCASE("admin") {
      auto roles = AccountRoles();
      roles.Set(AccountRole::ADMINISTRATOR);
      auto result =
        fixture.m_admin_client->SendRequest<LoadAccountsByRolesService>(roles);
      REQUIRE(result.size() == 2);
      REQUIRE(std::find(
        result.begin(), result.end(), fixture.m_admin_account) != result.end());
      REQUIRE(
        std::find(result.begin(), result.end(), admin_account) != result.end());
      roles.Set(AccountRole::SERVICE);
      result =
        fixture.m_admin_client->SendRequest<LoadAccountsByRolesService>(roles);
      REQUIRE(result.size() == 3);
      REQUIRE(std::find(
        result.begin(), result.end(), fixture.m_admin_account) != result.end());
      REQUIRE(
        std::find(result.begin(), result.end(), admin_account) != result.end());
      REQUIRE(std::find(
        result.begin(), result.end(), service_account) != result.end());
      roles.Unset(AccountRole::ADMINISTRATOR);
      result =
        fixture.m_admin_client->SendRequest<LoadAccountsByRolesService>(roles);
      REQUIRE(result.size() == 1);
      REQUIRE(result.front() == service_account);
    }
    SUBCASE("trader") {
      auto roles = AccountRoles();
      roles.Set(AccountRole::ADMINISTRATOR);
      auto result =
        fixture.m_trader_client->SendRequest<LoadAccountsByRolesService>(roles);
      REQUIRE(result.empty());
      roles.Set(AccountRole::SERVICE);
      result =
        fixture.m_trader_client->SendRequest<LoadAccountsByRolesService>(roles);
      REQUIRE(result.empty());
      roles.Unset(AccountRole::ADMINISTRATOR);
      result =
        fixture.m_trader_client->SendRequest<LoadAccountsByRolesService>(roles);
      REQUIRE(result.empty());
    }
  }

  TEST_CASE("load_administrators_root_entry") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto administrators_root = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "administrators");
    auto result = fixture.m_trader_client->SendRequest<
      LoadAdministratorsRootEntryService>();
    REQUIRE(result == administrators_root);
  }

  TEST_CASE("load_services_root_entry") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto services_root = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services");
    auto result = fixture.m_trader_client->SendRequest<
      LoadServicesRootEntryService>();
    REQUIRE(result == services_root);
  }

  TEST_CASE("load_trading_groups_root_entry") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto trading_groups_root = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "trading_groups");
    auto result = fixture.m_trader_client->SendRequest<
      LoadTradingGroupsRootEntryService>();
    REQUIRE(result == trading_groups_root);
  }

  TEST_CASE("check_administrator") {
    auto fixture = Fixture();
    auto is_admin = fixture.m_trader_client->SendRequest<
      CheckAdministratorService>(fixture.m_admin_account);
    REQUIRE(is_admin);
    is_admin = fixture.m_trader_client->SendRequest<CheckAdministratorService>(
      fixture.m_trader_account);
    REQUIRE(!is_admin);
  }

  TEST_CASE("load_account_roles") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto admin_roles = fixture.m_trader_client->SendRequest<
      LoadAccountRolesService>(fixture.m_admin_account);
    REQUIRE(admin_roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(!admin_roles.Test(AccountRole::SERVICE));
    REQUIRE(!admin_roles.Test(AccountRole::TRADER));
    REQUIRE(!admin_roles.Test(AccountRole::MANAGER));
    auto services_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services");
    auto service_account = fixture.make_account("service", services_group);
    auto service_roles = fixture.m_trader_client->SendRequest<
      LoadAccountRolesService>(service_account);
    REQUIRE(!service_roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(service_roles.Test(AccountRole::SERVICE));
    REQUIRE(!service_roles.Test(AccountRole::TRADER));
    REQUIRE(!service_roles.Test(AccountRole::MANAGER));
    auto trading_group = fixture.make_trading_group("test_trading_group");
    auto manager_account =
      fixture.make_account("manager43", trading_group.get_managers_directory());
    auto manager_roles = fixture.m_trader_client->SendRequest<
      LoadAccountRolesService>(manager_account);
    REQUIRE(!manager_roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(!manager_roles.Test(AccountRole::SERVICE));
    REQUIRE(!manager_roles.Test(AccountRole::TRADER));
    REQUIRE(manager_roles.Test(AccountRole::MANAGER));
    auto trader_account =
      fixture.make_account("trader96", trading_group.get_traders_directory());
    auto trader_roles = fixture.m_trader_client->SendRequest<
      LoadAccountRolesService>(trader_account);
    REQUIRE(!trader_roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(!trader_roles.Test(AccountRole::SERVICE));
    REQUIRE(trader_roles.Test(AccountRole::TRADER));
    REQUIRE(!trader_roles.Test(AccountRole::MANAGER));
  }

  TEST_CASE("load_supervised_account_roles") {
    auto fixture = Fixture();
    auto trading_group = fixture.make_trading_group("test_trading_group");
    auto manager_account =
      fixture.make_account("manager43", trading_group.get_managers_directory());
    auto trader_account =
      fixture.make_account("trader96", trading_group.get_traders_directory());
    auto roles =
      fixture.m_admin_client->SendRequest<LoadSupervisedAccountRolesService>(
        fixture.m_admin_account, trader_account);
    REQUIRE(roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(roles.Test(AccountRole::MANAGER));
    roles = fixture.m_admin_client->SendRequest<
      LoadSupervisedAccountRolesService>(manager_account, trader_account);
    REQUIRE(!roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(roles.Test(AccountRole::MANAGER));
    roles =
      fixture.m_admin_client->SendRequest<LoadSupervisedAccountRolesService>(
        manager_account, fixture.m_trader_account);
    REQUIRE(!roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(!roles.Test(AccountRole::MANAGER));
    roles =
      fixture.m_admin_client->SendRequest<LoadSupervisedAccountRolesService>(
        trader_account, manager_account);
    REQUIRE(!roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(!roles.Test(AccountRole::MANAGER));
    roles =
      fixture.m_admin_client->SendRequest<LoadSupervisedAccountRolesService>(
        fixture.m_admin_account, fixture.m_admin_account);
    REQUIRE(roles.Test(AccountRole::ADMINISTRATOR));
    REQUIRE(!roles.Test(AccountRole::MANAGER));
  }

  TEST_CASE("load_parent_trading_group") {
    auto fixture = Fixture();
    auto trading_group = fixture.make_trading_group("test_trading_group");
    auto trader_account =
      fixture.make_account("trader96", trading_group.get_traders_directory());
    auto parent_group =fixture.m_admin_client->SendRequest<
      LoadParentTradingGroupService>(trader_account);
    REQUIRE(parent_group == trading_group.get_entry());
    parent_group = fixture.m_admin_client->SendRequest<
      LoadParentTradingGroupService>(fixture.m_admin_account);
    REQUIRE(parent_group == DirectoryEntry());
  }

  TEST_CASE("load_account_identity") {
    auto fixture = Fixture();
    auto account =
      fixture.make_account("test_account", DirectoryEntry::GetStarDirectory());
    auto identity = AccountIdentity();
    identity.m_first_name = "Riley";
    identity.m_last_name = "Miller";
    fixture.m_data_store.store(account, identity);
    auto result = fixture.m_admin_client->SendRequest<
      LoadAccountIdentityService>(account);
    REQUIRE(result.m_first_name == identity.m_first_name);
    REQUIRE(result.m_last_name == identity.m_last_name);
  }

  TEST_CASE("store_account_identity") {
    auto fixture = Fixture();
    auto identity = AccountIdentity();
    identity.m_first_name = "Riley";
    identity.m_last_name = "Miller";
    auto account =
      fixture.make_account("test_account", DirectoryEntry::GetStarDirectory());
    SUBCASE("admin") {
      REQUIRE_NOTHROW(fixture.m_admin_client->SendRequest<
        StoreAccountIdentityService>(account, identity));
      auto result = fixture.m_admin_client->SendRequest<
        LoadAccountIdentityService>(account);
      REQUIRE(result.m_first_name == identity.m_first_name);
      REQUIRE(result.m_last_name == identity.m_last_name);
    }
    SUBCASE("trader") {
      REQUIRE_THROWS_AS(
        fixture.m_trader_client->SendRequest<StoreAccountIdentityService>(
          fixture.m_trader_account, identity), ServiceRequestException);
      REQUIRE_THROWS_AS(
        fixture.m_trader_client->SendRequest<StoreAccountIdentityService>(
          account, identity), ServiceRequestException);
    }
  }

  TEST_CASE("load_trading_group") {
    auto fixture = Fixture();
    auto trading_group = fixture.make_trading_group("test_trading_group");
    auto manager =
      fixture.make_account("manager44", trading_group.get_managers_directory());
    auto trader =
      fixture.make_account("trader96", trading_group.get_traders_directory());
    auto result = fixture.m_admin_client->SendRequest<LoadTradingGroupService>(
      trading_group.get_entry());
    REQUIRE(result.get_entry() == trading_group.get_entry());
    REQUIRE(result.get_managers_directory() ==
      trading_group.get_managers_directory());
    REQUIRE(result.get_traders_directory() ==
      trading_group.get_traders_directory());
    REQUIRE(result.get_managers().size() == 1);
    REQUIRE(result.get_managers().front() == manager);
    REQUIRE(result.get_traders().size() == 1);
    REQUIRE(result.get_traders().front() == trader);
  }

  TEST_CASE("load_administrators") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto admin_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "administrators");
    auto admin_account = fixture.make_account("admin2", admin_group);
    auto result =
      fixture.m_admin_client->SendRequest<LoadAdministratorsService>();
    REQUIRE(result.size() == 2);
    REQUIRE(std::find(result.begin(), result.end(), fixture.m_admin_account) !=
      result.end());
    REQUIRE(
      std::find(result.begin(), result.end(), admin_account) != result.end());
  }

  TEST_CASE("load_services") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto services_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services");
    auto service_account1 = fixture.make_account("service1", services_group);
    auto service_account2 = fixture.make_account("service2", services_group);
    auto result = fixture.m_admin_client->SendRequest<LoadServicesService>();
    REQUIRE(result.size() == 2);
    REQUIRE(std::find(result.begin(), result.end(), service_account1) !=
      result.end());
    REQUIRE(std::find(result.begin(), result.end(), service_account2) !=
      result.end());
  }

  TEST_CASE("load_entitlements") {
    auto fixture = Fixture();
    auto result =
      fixture.m_admin_client->SendRequest<LoadEntitlementsService>();
    REQUIRE(result.GetEntries().empty());
  }
}
