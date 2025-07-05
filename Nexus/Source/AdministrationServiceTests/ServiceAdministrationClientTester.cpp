#include <future>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/TestAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::MarketDataService;

namespace {
  using OperationQueue =
    Queue<std::shared_ptr<TestAdministrationClient::Operation>>;
  using TestServiceAdministrationClient =
    ServiceAdministrationClient<TestServiceProtocolClientBuilder>;

  template<typename O, typename F, typename R, typename E>
  void require_operation(TestServiceAdministrationClient& client,
      OperationQueue& operations, F&& f, const R& expected, E&& e) {
    auto future = std::async(std::launch::async, [&] {
      return std::forward<F>(f)(client);
    });
    auto operation = operations.Pop();
    auto specific = std::get_if<O>(&*operation);
    REQUIRE(specific);
    specific->m_result.set(expected);
    std::forward<E>(e)(std::move(future).get());
  }

  template<typename O, typename F>
  void require_operation(TestServiceAdministrationClient& client,
      OperationQueue& operations, F&& f) {
    auto future = std::async(std::launch::async, [&] {
      return std::forward<F>(f)(client);
    });
    auto operation = operations.Pop();
    auto specific = std::get_if<O>(&*operation);
    REQUIRE(specific);
    specific->m_result.set();
  }

  template<typename O, typename F, typename R>
  void require_operation(TestServiceAdministrationClient& client,
      OperationQueue& operations, F&& f, const R& expected) {
    require_operation<O>(client, operations, std::forward<F>(f), expected,
      [&] (const auto& result) {
        REQUIRE(result == expected);
      });
  }

  struct Fixture {
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::shared_ptr<OperationQueue> m_operations;

    Fixture()
      : m_server_connection(std::make_shared<TestServerConnection>()),
        m_server(m_server_connection,
          factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()),
        m_operations(std::make_shared<OperationQueue>()) {
      RegisterAdministrationServices(Store(m_server.GetSlots()));
      RegisterAdministrationMessages(Store(m_server.GetSlots()));
      LoadAccountsByRolesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_accounts_by_roles, this));
      LoadAdministratorsRootEntryService::AddRequestSlot(
        Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_administrators_root_entry, this));
      LoadServicesRootEntryService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_services_root_entry, this));
      LoadTradingGroupsRootEntryService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_trading_groups_root_entry, this));
      CheckAdministratorService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_check_administrator, this));
      LoadAccountRolesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_account_roles, this));
      LoadSupervisedAccountRolesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_supervised_account_roles, this));
      LoadParentTradingGroupService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_parent_trading_group, this));
      LoadAccountIdentityService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_identity, this));
      StoreAccountIdentityService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_store_identity, this));
      LoadTradingGroupService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_trading_group, this));
      LoadManagedTradingGroupsService::AddRequestSlot(
        Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_managed_trading_groups, this));
      LoadAdministratorsService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_administrators, this));
      LoadServicesService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_services, this));
      LoadEntitlementsService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_entitlements, this));
      LoadAccountEntitlementsService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_load_account_entitlements, this));
      StoreEntitlementsService::AddRequestSlot(Store(m_server.GetSlots()),
        std::bind_front(&Fixture::on_store_entitlements, this));
    }

    template<typename T>
    using Request =
      RequestToken<TestServiceProtocolServer::ServiceProtocolClient, T>;

    std::unique_ptr<TestServiceAdministrationClient> make_client() {
      auto builder = TestServiceProtocolClientBuilder([&] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<
        TestServiceProtocolClientBuilder::Timer>>());
      return std::make_unique<TestServiceAdministrationClient>(builder);
    }

    void on_load_accounts_by_roles(
        Request<LoadAccountsByRolesService>& request, AccountRoles roles) {
      auto async = Async<std::vector<DirectoryEntry>>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadAccountsByRolesOperation>, roles,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(std::move(result));
    }

    void on_load_administrators_root_entry(
        Request<LoadAdministratorsRootEntryService>& request) {
      auto async = Async<DirectoryEntry>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadAdministratorsRootEntryOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(std::move(result));
    }

    void on_load_services_root_entry(
        Request<LoadServicesRootEntryService>& request) {
      auto async = Async<DirectoryEntry>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadServicesRootEntryOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(std::move(result));
    }

    void on_load_trading_groups_root_entry(
        Request<LoadTradingGroupsRootEntryService>& request) {
      auto async = Async<DirectoryEntry>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadTradingGroupsRootEntryOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(std::move(result));
    }

    void on_check_administrator(Request<CheckAdministratorService>& request,
        const DirectoryEntry& account) {
      auto async = Async<bool>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::CheckAdministratorOperation>, account,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_account_roles(Request<LoadAccountRolesService>& request,
        const DirectoryEntry& account) {
      auto async = Async<AccountRoles>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<TestAdministrationClient::LoadAccountRolesOperation>,
        account, async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_supervised_account_roles(
        Request<LoadSupervisedAccountRolesService>& request,
        const DirectoryEntry& parent, const DirectoryEntry& child) {
      auto async = Async<AccountRoles>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadParentChildAccountRolesOperation>,
        parent, child, async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_parent_trading_group(
        Request<LoadParentTradingGroupService>& request,
        const DirectoryEntry& account) {
      auto async = Async<DirectoryEntry>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadParentTradingGroupOperation>, account,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_identity(Request<LoadAccountIdentityService>& request,
        const DirectoryEntry& account) {
      auto async = Async<AccountIdentity>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<TestAdministrationClient::LoadIdentityOperation>,
        account, async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_store_identity(Request<StoreAccountIdentityService>& request,
        const DirectoryEntry& account, const AccountIdentity& identity) {
      auto async = Async<void>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<TestAdministrationClient::StoreIdentityOperation>,
        account, identity, async.GetEval());
      m_operations->Push(operation);
      async.Get();
      request.SetResult();
    }

    void on_load_trading_group(Request<LoadTradingGroupService>& request,
        const DirectoryEntry& directory) {
      auto async = Async<TradingGroup>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<TestAdministrationClient::LoadTradingGroupOperation>,
        directory, async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_managed_trading_groups(
        Request<LoadManagedTradingGroupsService>& request,
        const DirectoryEntry& account) {
      auto async = Async<std::vector<DirectoryEntry>>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadManagedTradingGroupsOperation>,
        account, async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_administrators(Request<LoadAdministratorsService>& request) {
      auto async = Async<std::vector<DirectoryEntry>>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadAdministratorsOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_services(Request<LoadServicesService>& request) {
      auto async = Async<std::vector<DirectoryEntry>>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<TestAdministrationClient::LoadServicesOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_entitlements(Request<LoadEntitlementsService>& request) {
      auto async = Async<EntitlementDatabase>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<TestAdministrationClient::LoadEntitlementsOperation>,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_load_account_entitlements(
        Request<LoadAccountEntitlementsService>& request,
        const DirectoryEntry& account) {
      auto async = Async<std::vector<DirectoryEntry>>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::LoadAccountEntitlementsOperation>, account,
        async.GetEval());
      m_operations->Push(operation);
      auto result = std::move(async.Get());
      request.SetResult(result);
    }

    void on_store_entitlements(Request<StoreEntitlementsService>& request,
        const DirectoryEntry& account,
        const std::vector<DirectoryEntry>& entitlements) {
      auto async = Async<void>();
      auto operation = std::make_shared<TestAdministrationClient::Operation>(
        std::in_place_type<
          TestAdministrationClient::StoreEntitlementsOperation>, account,
        entitlements, async.GetEval());
      m_operations->Push(operation);
      async.Get();
      request.SetResult();
    }
  };
}

TEST_SUITE("ServiceAdministrationClient") {
  TEST_CASE("load_accounts_by_roles") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto roles = AccountRoles();
    auto accounts = std::vector{DirectoryEntry::MakeAccount(1, "test_account")};
    require_operation<TestAdministrationClient::LoadAccountsByRolesOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_accounts_by_roles(roles);
      }, accounts);
  }

  TEST_CASE("load_administrators_root_entry") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto entry = DirectoryEntry::MakeAccount(1, "admin_root");
    require_operation<
      TestAdministrationClient::LoadAdministratorsRootEntryOperation>(
        *client, *fixture.m_operations, [&] (auto& client) {
          return client.load_administrators_root_entry();
        }, entry);
  }

  TEST_CASE("load_services_root_entry") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto entry = DirectoryEntry::MakeAccount(2, "services_root");
    require_operation<TestAdministrationClient::LoadServicesRootEntryOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_services_root_entry();
      }, entry);
  }

  TEST_CASE("load_trading_groups_root_entry") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto entry = DirectoryEntry::MakeAccount(3, "trading_groups_root");
    require_operation<TestAdministrationClient::LoadTradingGroupsRootEntryOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_trading_groups_root_entry();
      }, entry);
  }

  TEST_CASE("check_administrator") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(4, "admin_account");
    require_operation<TestAdministrationClient::CheckAdministratorOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.check_administrator(account);
      }, true);
  }

  TEST_CASE("load_account_roles") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(5, "account");
    auto roles = AccountRoles();
    roles.Set(AccountRole::MANAGER);
    require_operation<TestAdministrationClient::LoadAccountRolesOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_account_roles(account);
      }, roles);
  }

  TEST_CASE("load_supervised_account_roles") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto parent = DirectoryEntry::MakeAccount(6, "parent_account");
    auto child = DirectoryEntry::MakeAccount(7, "child_account");
    auto roles = AccountRoles();
    roles.Set(AccountRole::TRADER);
    require_operation<
      TestAdministrationClient::LoadParentChildAccountRolesOperation>(
        *client, *fixture.m_operations, [&] (auto& client) {
          return client.load_account_roles(parent, child);
        }, roles);
  }

  TEST_CASE("load_parent_trading_group") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(8, "trader_account");
    auto parent_group = DirectoryEntry::MakeAccount(9, "parent_group");
    require_operation<
      TestAdministrationClient::LoadParentTradingGroupOperation>(
        *client, *fixture.m_operations, [&] (auto& client) {
          return client.load_parent_trading_group(account);
        }, parent_group);
  }

  TEST_CASE("load_identity") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(10, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "John";
    identity.m_last_name = "Doe";
    require_operation<TestAdministrationClient::LoadIdentityOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_identity(account);
      }, identity,
      [&] (const auto& received) {
        REQUIRE(identity.m_first_name == received.m_first_name);
        REQUIRE(identity.m_last_name == received.m_last_name);
      });
  }

  TEST_CASE("store_identity") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(11, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "Jane";
    identity.m_last_name = "Smith";
    require_operation<TestAdministrationClient::StoreIdentityOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        client.store_identity(account, identity);
      });
  }

  TEST_CASE("load_trading_group") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto directory = DirectoryEntry::MakeAccount(1, "trading_group");
    auto trading_group = TradingGroup(directory,
      DirectoryEntry::MakeAccount(2, "managers"),
      {DirectoryEntry::MakeAccount(3, "manager1")},
      DirectoryEntry::MakeAccount(4, "traders"),
      {DirectoryEntry::MakeAccount(5, "trader1")});
    require_operation<TestAdministrationClient::LoadTradingGroupOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_trading_group(directory);
      }, trading_group,
      [&] (const auto& received) {
        REQUIRE(received.get_entry() == trading_group.get_entry());
        REQUIRE(received.get_managers_directory() ==
          trading_group.get_managers_directory());
        REQUIRE(received.get_managers() == trading_group.get_managers());
        REQUIRE(received.get_traders_directory() ==
          trading_group.get_traders_directory());
        REQUIRE(received.get_traders() == trading_group.get_traders());
      });
  }

  TEST_CASE("load_managed_trading_groups") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(6, "manager_account");
    auto managed_groups = std::vector{DirectoryEntry::MakeAccount(7, "group1"),
      DirectoryEntry::MakeAccount(8, "group2")};
    require_operation<
      TestAdministrationClient::LoadManagedTradingGroupsOperation>(
        *client, *fixture.m_operations, [&] (auto& client) {
          return client.load_managed_trading_groups(account);
        }, managed_groups);
  }

  TEST_CASE("load_administrators") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto administrators = std::vector{DirectoryEntry::MakeAccount(9, "admin1"),
      DirectoryEntry::MakeAccount(10, "admin2")};
    require_operation<TestAdministrationClient::LoadAdministratorsOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_administrators();
      }, administrators);
  }

  TEST_CASE("load_services") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto services = std::vector{DirectoryEntry::MakeAccount(11, "service1"),
      DirectoryEntry::MakeAccount(12, "service2")};
    require_operation<TestAdministrationClient::LoadServicesOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_services();
      }, services);
  }

  TEST_CASE("load_entitlements") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto entitlements = EntitlementDatabase();
    entitlements.Add({"Entitlement1", Money(100), USD,
      DirectoryEntry::MakeAccount(1, "group1"), {{EntitlementKey(NYSE),
        MarketDataTypeSet({MarketDataType::BBO_QUOTE})}}});
    entitlements.Add({"Entitlement2", Money(200), CAD,
      DirectoryEntry::MakeAccount(2, "group2"), {{EntitlementKey(TSX),
        MarketDataTypeSet({MarketDataType::BOOK_QUOTE})}}});
    require_operation<TestAdministrationClient::LoadEntitlementsOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        return client.load_entitlements();
      }, entitlements,
      [&] (const auto& received) {
        REQUIRE(received.GetEntries() == entitlements.GetEntries());
      });
  }

  TEST_CASE("load_account_entitlements") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(13, "entitled_account");
    auto entitlements = std::vector{
      DirectoryEntry::MakeAccount(14, "entitlement1"),
      DirectoryEntry::MakeAccount(15, "entitlement2")};
    require_operation<
      TestAdministrationClient::LoadAccountEntitlementsOperation>(
        *client, *fixture.m_operations, [&] (auto& client) {
          return client.load_entitlements(account);
        }, entitlements);
  }

  TEST_CASE("store_entitlements") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto account = DirectoryEntry::MakeAccount(16, "entitled_account");
    auto entitlements = std::vector{
      DirectoryEntry::MakeAccount(17, "entitlement1"),
      DirectoryEntry::MakeAccount(18, "entitlement2")};
    require_operation<TestAdministrationClient::StoreEntitlementsOperation>(
      *client, *fixture.m_operations, [&] (auto& client) {
        client.store_entitlements(account, entitlements);
      });
  }
}
