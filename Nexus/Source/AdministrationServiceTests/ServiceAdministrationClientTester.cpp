#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;
using namespace Nexus::RiskService;

namespace {
  struct Fixture {
    using TestServiceAdministrationClient =
      ServiceAdministrationClient<TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceAdministrationClient> m_client;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterAdministrationServices(Store(m_server.GetSlots()));
      RegisterAdministrationMessages(Store(m_server.GetSlots()));
      auto builder = TestServiceProtocolClientBuilder([=] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<
        TestServiceProtocolClientBuilder::Timer>>());
      m_client = std::make_unique<TestServiceAdministrationClient>(builder);
    }

    template<typename T, typename F>
    void handle(F&& handler) {
      T::AddRequestSlot(Store(m_server.GetSlots()),
        [handler = std::forward<F>(handler)] (auto&&... args) {
          try {
            handler(std::forward<decltype(args)>(args)...);
          } catch(...) {
            throw ServiceRequestException("Test failed.");
          }
        });
    }
  };
}

#define REQUIRE_NO_THROW(expression) \
  [&] { \
    REQUIRE_NOTHROW(return (expression)); \
    throw 0; \
  }()

TEST_SUITE("ServiceAdministrationClient") {
  TEST_CASE("load_accounts_by_roles") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(16, "entitled_account");
    auto roles = AccountRoles();
    auto accounts = std::vector{DirectoryEntry::MakeAccount(1, "test_account")};
    fixture.handle<LoadAccountsByRolesService>(
      [&] (auto& request, const auto& received_roles) {
        REQUIRE(received_roles == roles);
        request.SetResult(accounts);
      });
    auto received_accounts = REQUIRE_NO_THROW(
      fixture.m_client->load_accounts_by_roles(roles));
    REQUIRE(received_accounts == accounts);
  }

  TEST_CASE("load_administrators_root_entry") {
    auto fixture = Fixture();
    auto entry = DirectoryEntry::MakeDirectory(1, "admin_root");
    fixture.handle<LoadAdministratorsRootEntryService>([&] (auto& request) {
      request.SetResult(entry);
    });
    auto received_entry =
      REQUIRE_NO_THROW(fixture.m_client->load_administrators_root_entry());
    REQUIRE(received_entry == entry);
  }

  TEST_CASE("load_services_root_entry") {
    auto fixture = Fixture();
    auto entry = DirectoryEntry::MakeDirectory(2, "services_root");
    fixture.handle<LoadServicesRootEntryService>([&] (auto& request) {
      request.SetResult(entry);
    });
    auto received_entry =
      REQUIRE_NO_THROW(fixture.m_client->load_services_root_entry());
    REQUIRE(received_entry == entry);
  }

  TEST_CASE("load_trading_groups_root_entry") {
    auto fixture = Fixture();
    auto entry = DirectoryEntry::MakeDirectory(3, "trading_groups_root");
    fixture.handle<LoadTradingGroupsRootEntryService>([&] (auto& request) {
      request.SetResult(entry);
    });
    auto received_entry =
      REQUIRE_NO_THROW(fixture.m_client->load_trading_groups_root_entry());
    REQUIRE(received_entry == entry);
  }

  TEST_CASE("check_administrator") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(4, "admin_account");
    fixture.handle<CheckAdministratorService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.SetResult(true);
      });
    auto is_administrator =
      REQUIRE_NO_THROW(fixture.m_client->check_administrator(account));
    REQUIRE(is_administrator);
  }

  TEST_CASE("load_account_roles") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(5, "account");
    auto roles = AccountRoles();
    roles.Set(AccountRole::MANAGER);
    fixture.handle<LoadAccountRolesService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.SetResult(roles);
      });
    auto received_roles =
      REQUIRE_NO_THROW(fixture.m_client->load_account_roles(account));
    REQUIRE(received_roles == roles);
  }

  TEST_CASE("load_supervised_account_roles") {
    auto fixture = Fixture();
    auto parent = DirectoryEntry::MakeAccount(6, "parent_account");
    auto child = DirectoryEntry::MakeAccount(7, "child_account");
    auto roles = AccountRoles();
    roles.Set(AccountRole::TRADER);
    fixture.handle<LoadSupervisedAccountRolesService>(
      [&] (auto& request, const auto& received_parent,
          const auto& received_child) {
        REQUIRE(received_parent == parent);
        REQUIRE(received_child == child);
        request.SetResult(roles);
      });
    auto received_roles =
      REQUIRE_NO_THROW(fixture.m_client->load_account_roles(parent, child));
    REQUIRE(received_roles == roles);
  }

  TEST_CASE("load_parent_trading_group") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(8, "trader_account");
    auto parent_group = DirectoryEntry::MakeAccount(9, "parent_group");
    fixture.handle<LoadParentTradingGroupService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.SetResult(parent_group);
      });
    auto received_parent_group =
      REQUIRE_NO_THROW(fixture.m_client->load_parent_trading_group(account));
    REQUIRE(received_parent_group == parent_group);
  }

  TEST_CASE("load_identity") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(10, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "John";
    identity.m_last_name = "Doe";
    fixture.handle<LoadAccountIdentityService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.SetResult(identity);
      });
    auto received_identity =
      REQUIRE_NO_THROW(fixture.m_client->load_identity(account));
    REQUIRE(received_identity.m_first_name == identity.m_first_name);
    REQUIRE(received_identity.m_last_name == identity.m_last_name);
  }

  TEST_CASE("store_identity") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(11, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "Jane";
    identity.m_last_name = "Smith";
    fixture.handle<StoreAccountIdentityService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_identity) {
        REQUIRE(received_account == account);
        REQUIRE(received_identity.m_first_name == identity.m_first_name);
        REQUIRE(received_identity.m_last_name == identity.m_last_name);
        request.SetResult();
      });
    REQUIRE_NOTHROW(fixture.m_client->store_identity(account, identity));
  }

  TEST_CASE("load_trading_group") {
    auto fixture = Fixture();
    auto directory = DirectoryEntry::MakeDirectory(1, "trading_group");
    auto trading_group = TradingGroup(directory,
      DirectoryEntry::MakeDirectory(2, "managers"),
      {DirectoryEntry::MakeAccount(3, "manager1")},
      DirectoryEntry::MakeDirectory(4, "traders"),
      {DirectoryEntry::MakeAccount(5, "trader1")});
    fixture.handle<LoadTradingGroupService>(
      [&] (auto& request, const auto& received_directory) {
        REQUIRE(received_directory == directory);
        request.SetResult(trading_group);
      });
    auto received_trading_group =
      REQUIRE_NO_THROW(fixture.m_client->load_trading_group(directory));
    REQUIRE(received_trading_group.get_entry() == trading_group.get_entry());
    REQUIRE(received_trading_group.get_managers_directory() ==
      trading_group.get_managers_directory());
    REQUIRE(
      received_trading_group.get_managers() == trading_group.get_managers());
    REQUIRE(received_trading_group.get_traders_directory() ==
      trading_group.get_traders_directory());
    REQUIRE(
      received_trading_group.get_traders() == trading_group.get_traders());
  }

  TEST_CASE("load_managed_trading_groups") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(6, "manager_account");
    auto managed_groups = std::vector{DirectoryEntry::MakeAccount(7, "group1"),
      DirectoryEntry::MakeAccount(8, "group2")};
    fixture.handle<LoadManagedTradingGroupsService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.SetResult(managed_groups);
      });
    auto received_groups =
      REQUIRE_NO_THROW(fixture.m_client->load_managed_trading_groups(account));
    REQUIRE(received_groups == managed_groups);
  }

  TEST_CASE("load_administrators") {
    auto fixture = Fixture();
    auto administrators = std::vector{DirectoryEntry::MakeAccount(9, "admin1"),
      DirectoryEntry::MakeAccount(10, "admin2")};
    fixture.handle<LoadAdministratorsService>([&] (auto& request) {
      request.SetResult(administrators);
    });
    auto received_administrators =
      REQUIRE_NO_THROW(fixture.m_client->load_administrators());
    REQUIRE(received_administrators == administrators);
  }

  TEST_CASE("load_services") {
    auto fixture = Fixture();
    auto services = std::vector{DirectoryEntry::MakeAccount(11, "service1"),
      DirectoryEntry::MakeAccount(12, "service2")};
    fixture.handle<LoadServicesService>([&] (auto& request) {
      request.SetResult(services);
    });
    auto received_services =
      REQUIRE_NO_THROW(fixture.m_client->load_services());
    REQUIRE(received_services == services);
  }

  TEST_CASE("load_entitlements") {
    auto fixture = Fixture();
    auto entitlements = EntitlementDatabase();
    entitlements.Add({"Entitlement1", Money(100), USD,
      DirectoryEntry::MakeAccount(1, "group1"), {{EntitlementKey(NYSE),
        MarketDataTypeSet({MarketDataType::BBO_QUOTE})}}});
    entitlements.Add({"Entitlement2", Money(200), CAD,
      DirectoryEntry::MakeAccount(2, "group2"), {{EntitlementKey(TSX),
        MarketDataTypeSet({MarketDataType::BOOK_QUOTE})}}});
    fixture.handle<LoadEntitlementsService>([&] (auto& request) {
      request.SetResult(entitlements);
    });
    auto received_entitlements =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlements());
    REQUIRE(received_entitlements.GetEntries() == entitlements.GetEntries());
  }

  TEST_CASE("load_account_entitlements") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(13, "entitled_account");
    auto entitlements = std::vector{
      DirectoryEntry::MakeAccount(14, "entitlement1"),
      DirectoryEntry::MakeAccount(15, "entitlement2")};
    fixture.handle<LoadAccountEntitlementsService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.SetResult(entitlements);
      });
    auto received_entitlements =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlements(account));
    REQUIRE(received_entitlements == entitlements);
  }

  TEST_CASE("store_entitlements") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(16, "entitled_account");
    auto entitlements = std::vector{
      DirectoryEntry::MakeAccount(17, "entitlement1"),
      DirectoryEntry::MakeAccount(18, "entitlement2")};
    fixture.handle<StoreEntitlementsService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_entitlements) {
        REQUIRE(received_account == account);
        REQUIRE(received_entitlements == entitlements);
        request.SetResult();
      });
    REQUIRE_NOTHROW(
      fixture.m_client->store_entitlements(account, entitlements));
  }

  TEST_CASE("risk_parameters") {
    auto fixture = Fixture();
    auto account_a = DirectoryEntry::MakeAccount(55, "Alexis");
    auto parameters_a = std::make_shared<Queue<RiskParameters>>();
    fixture.handle<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_a);
        request.SetResult(RiskParameters());
      });
    REQUIRE_NO_THROW(fixture.m_client->get_risk_parameters_publisher(
      account_a).Monitor(parameters_a));
  }
}
