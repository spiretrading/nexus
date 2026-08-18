#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/Queries/AccountModificationRequestAccessor.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Venues;

namespace {
  struct InterceptingDataStore : LocalAdministrationDataStore {
    std::function<void ()> m_on_load_entitlement_modification;
    std::function<void ()> m_on_load_status;

    AccountModificationRequest::Update load_account_modification_request_status(
        AccountModificationRequest::Id id) {
      if(m_on_load_status) {
        auto callback = std::exchange(m_on_load_status, nullptr);
        callback();
      }
      return LocalAdministrationDataStore::
        load_account_modification_request_status(id);
    }

    EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id) {
      if(m_on_load_entitlement_modification) {
        auto callback =
          std::exchange(m_on_load_entitlement_modification, nullptr);
        callback();
      }
      return LocalAdministrationDataStore::load_entitlement_modification(id);
    }
  };

  template<typename D = LocalAdministrationDataStore>
  struct Fixture {
    using DataStore = D;
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaAdministrationServlet<
        ServiceLocatorClient, DataStore*, FixedTimeClient*, TriggerTimer*>>;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    DataStore m_data_store;
    EntitlementDatabase m_entitlements;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    TradingGroup m_trading_group;
    DirectoryEntry m_admin_account;
    std::unique_ptr<TestServiceProtocolClient> m_admin_client;
    DirectoryEntry m_manager_account;
    std::unique_ptr<TestServiceProtocolClient> m_manager_client;
    DirectoryEntry m_trader_account;
    std::unique_ptr<TestServiceProtocolClient> m_trader_client;

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.get_root().make_account(
        name, "", parent);
    }

    auto make_trading_group(const std::string& name) {
      auto& service_locator = m_service_locator_environment.get_root();
      auto trading_groups_root = service_locator.load_directory_entry(
        DirectoryEntry::STAR_DIRECTORY, "trading_groups");
      auto trading_group =
        service_locator.make_directory(name, trading_groups_root);
      auto managers_group =
        service_locator.make_directory("managers", trading_group);
      auto traders_group =
        service_locator.make_directory("traders", trading_group);
      return TradingGroup(trading_group, managers_group, {}, traders_group, {});
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.make_client(name, "");
      auto authenticator = SessionAuthenticator(Ref(service_locator_client));
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        std::make_unique<LocalClientChannel>(name, *m_server_connection),
        init());
      Nexus::register_query_types(
        out(protocol_client->get_slots().get_registry()));
      register_administration_services(out(protocol_client->get_slots()));
      register_administration_messages(out(protocol_client->get_slots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.get_account(), std::move(protocol_client));
    }

    Fixture()
        : m_time_client(time_from_string("2024-07-04 12:00:00")),
          m_server_connection(std::make_shared<LocalServerConnection>()) {
      auto entitlement1 =
        m_service_locator_environment.get_root().make_directory(
          "entitlement1", DirectoryEntry::STAR_DIRECTORY);
      auto entitlement2 =
        m_service_locator_environment.get_root().make_directory(
          "entitlement2", DirectoryEntry::STAR_DIRECTORY);
      m_entitlements.add({"Entitlement1", Money(100), AUD, entitlement1,
        {{EntitlementKey(ASX),
          MarketDataTypeSet({MarketDataType::BBO_QUOTE})}}});
      m_entitlements.add({"Entitlement2", Money(200), CAD, entitlement2,
        {{EntitlementKey(TSX),
          MarketDataTypeSet({MarketDataType::BOOK_QUOTE})}}});
      auto servlet_account =
        make_account("administration_service", DirectoryEntry::STAR_DIRECTORY);
      m_service_locator_environment.get_root().store(
        servlet_account, DirectoryEntry::STAR_DIRECTORY, Permissions(~0));
      m_servlet_service_locator_client.emplace(
        m_service_locator_environment.make_client(servlet_account.m_name, ""));
      m_container.emplace(init(*m_servlet_service_locator_client,
        init(&m_service_locator_environment.get_root(), m_entitlements,
          &m_data_store, &m_time_client, &m_timer)), m_server_connection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_trading_group = make_trading_group("trading_group");
      auto admin_group =
        m_service_locator_environment.get_root().load_directory_entry(
          DirectoryEntry::STAR_DIRECTORY, "administrators");
      make_account("admin", admin_group);
      std::tie(m_admin_account, m_admin_client) = make_client("admin");
      make_account("manager", m_trading_group.get_managers_directory());
      std::tie(m_manager_account, m_manager_client) = make_client("manager");
      make_account("trader", m_trading_group.get_traders_directory());
      std::tie(m_trader_account, m_trader_client) = make_client("trader");
    }
  };
}

TEST_SUITE("AdministrationServlet") {
  TEST_CASE("directories_initialized") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    REQUIRE_NOTHROW(service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "administrators"));
    REQUIRE_NOTHROW(service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "services"));
    REQUIRE_NOTHROW(service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "trading_groups"));
  }

  TEST_CASE("load_accounts_by_roles") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto admin_group = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "administrators");
    auto services_group = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "services");
    auto admin_account = fixture.make_account("b", admin_group);
    auto service_account = fixture.make_account("c", services_group);
    SUBCASE("admin") {
      auto roles = AccountRoles();
      roles.set(AccountRole::ADMINISTRATOR);
      auto result =
        fixture.m_admin_client->send_request<LoadAccountsByRolesService>(roles);
      REQUIRE(result.size() == 2);
      REQUIRE(std::find(
        result.begin(), result.end(), fixture.m_admin_account) != result.end());
      REQUIRE(
        std::find(result.begin(), result.end(), admin_account) != result.end());
      roles.set(AccountRole::SERVICE);
      result =
        fixture.m_admin_client->send_request<LoadAccountsByRolesService>(roles);
      REQUIRE(result.size() == 3);
      REQUIRE(std::find(
        result.begin(), result.end(), fixture.m_admin_account) != result.end());
      REQUIRE(
        std::find(result.begin(), result.end(), admin_account) != result.end());
      REQUIRE(std::find(
        result.begin(), result.end(), service_account) != result.end());
      roles.reset(AccountRole::ADMINISTRATOR);
      result =
        fixture.m_admin_client->send_request<LoadAccountsByRolesService>(roles);
      REQUIRE(result.size() == 1);
      REQUIRE(result.front() == service_account);
    }
    SUBCASE("trader") {
      auto roles = AccountRoles();
      roles.set(AccountRole::ADMINISTRATOR);
      auto result = fixture.m_trader_client->send_request<
        LoadAccountsByRolesService>(roles);
      REQUIRE(result.empty());
      roles.set(AccountRole::SERVICE);
      result = fixture.m_trader_client->send_request<
        LoadAccountsByRolesService>(roles);
      REQUIRE(result.empty());
      roles.reset(AccountRole::ADMINISTRATOR);
      result = fixture.m_trader_client->send_request<
        LoadAccountsByRolesService>(roles);
      REQUIRE(result.empty());
    }
  }

  TEST_CASE("load_administrators_root_entry") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto administrators_root = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "administrators");
    auto result = fixture.m_trader_client->send_request<
      LoadAdministratorsRootEntryService>();
    REQUIRE(result == administrators_root);
  }

  TEST_CASE("load_services_root_entry") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto services_root = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "services");
    auto result = fixture.m_trader_client->send_request<
      LoadServicesRootEntryService>();
    REQUIRE(result == services_root);
  }

  TEST_CASE("load_trading_groups_root_entry") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto trading_groups_root = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "trading_groups");
    auto result = fixture.m_trader_client->send_request<
      LoadTradingGroupsRootEntryService>();
    REQUIRE(result == trading_groups_root);
  }

  TEST_CASE("check_administrator") {
    auto fixture = Fixture();
    auto is_admin = fixture.m_trader_client->send_request<
      CheckAdministratorService>(fixture.m_admin_account);
    REQUIRE(is_admin);
    is_admin = fixture.m_trader_client->send_request<CheckAdministratorService>(
      fixture.m_trader_account);
    REQUIRE(!is_admin);
  }

  TEST_CASE("load_account_roles") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto admin_roles = fixture.m_admin_client->send_request<
      LoadAccountRolesService>(fixture.m_admin_account);
    REQUIRE(admin_roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(!admin_roles.test(AccountRole::SERVICE));
    REQUIRE(!admin_roles.test(AccountRole::TRADER));
    REQUIRE(!admin_roles.test(AccountRole::MANAGER));
    auto services_group = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "services");
    auto service_account = fixture.make_account("service", services_group);
    auto service_roles = fixture.m_admin_client->send_request<
      LoadAccountRolesService>(service_account);
    REQUIRE(!service_roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(service_roles.test(AccountRole::SERVICE));
    REQUIRE(!service_roles.test(AccountRole::TRADER));
    REQUIRE(!service_roles.test(AccountRole::MANAGER));
    auto manager_roles = fixture.m_manager_client->send_request<
      LoadAccountRolesService>(fixture.m_manager_account);
    REQUIRE(!manager_roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(!manager_roles.test(AccountRole::SERVICE));
    REQUIRE(!manager_roles.test(AccountRole::TRADER));
    REQUIRE(manager_roles.test(AccountRole::MANAGER));
    auto trader_roles = fixture.m_trader_client->send_request<
      LoadAccountRolesService>(fixture.m_trader_account);
    REQUIRE(!trader_roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(!trader_roles.test(AccountRole::SERVICE));
    REQUIRE(trader_roles.test(AccountRole::TRADER));
    REQUIRE(!trader_roles.test(AccountRole::MANAGER));
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      LoadAccountRolesService>(fixture.m_admin_account),
      ServiceRequestException);
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      LoadAccountRolesService>(service_account), ServiceRequestException);
  }

  TEST_CASE("load_supervised_account_roles") {
    auto fixture = Fixture();
    auto roles =
      fixture.m_admin_client->send_request<LoadSupervisedAccountRolesService>(
        fixture.m_admin_account, fixture.m_trader_account);
    REQUIRE(roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(roles.test(AccountRole::MANAGER));
    roles =
      fixture.m_admin_client->send_request<LoadSupervisedAccountRolesService>(
        fixture.m_manager_account, fixture.m_trader_account);
    REQUIRE(!roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(roles.test(AccountRole::MANAGER));
    roles =
      fixture.m_admin_client->send_request<LoadSupervisedAccountRolesService>(
        fixture.m_trader_account, fixture.m_manager_account);
    REQUIRE(!roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(!roles.test(AccountRole::MANAGER));
    roles =
      fixture.m_admin_client->send_request<LoadSupervisedAccountRolesService>(
        fixture.m_admin_account, fixture.m_admin_account);
    REQUIRE(roles.test(AccountRole::ADMINISTRATOR));
    REQUIRE(!roles.test(AccountRole::MANAGER));
  }

  TEST_CASE("load_parent_trading_group") {
    auto fixture = Fixture();
    auto parent_group = fixture.m_admin_client->send_request<
      LoadParentTradingGroupService>(fixture.m_trader_account);
    REQUIRE(parent_group == fixture.m_trading_group.get_entry());
    parent_group = fixture.m_admin_client->send_request<
      LoadParentTradingGroupService>(fixture.m_admin_account);
    REQUIRE(parent_group == DirectoryEntry());
  }

  TEST_CASE("load_account_identity") {
    auto fixture = Fixture();
    auto account =
      fixture.make_account("test_account", DirectoryEntry::STAR_DIRECTORY);
    auto identity = AccountIdentity();
    identity.m_first_name = "Riley";
    identity.m_last_name = "Miller";
    fixture.m_data_store.store(account, identity);
    auto result = fixture.m_admin_client->send_request<
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
      fixture.make_account("test_account", DirectoryEntry::STAR_DIRECTORY);
    SUBCASE("admin") {
      REQUIRE_NOTHROW(fixture.m_admin_client->send_request<
        StoreAccountIdentityService>(account, identity));
      auto result = fixture.m_admin_client->send_request<
        LoadAccountIdentityService>(account);
      REQUIRE(result.m_first_name == identity.m_first_name);
      REQUIRE(result.m_last_name == identity.m_last_name);
    }
    SUBCASE("trader") {
      REQUIRE_THROWS_AS(
        fixture.m_trader_client->send_request<StoreAccountIdentityService>(
          fixture.m_trader_account, identity), ServiceRequestException);
      REQUIRE_THROWS_AS(
        fixture.m_trader_client->send_request<StoreAccountIdentityService>(
          account, identity), ServiceRequestException);
    }
  }

  TEST_CASE("load_trading_group") {
    auto fixture = Fixture();
    auto result = fixture.m_admin_client->send_request<LoadTradingGroupService>(
      fixture.m_trading_group.get_entry());
    REQUIRE(result.get_entry() == fixture.m_trading_group.get_entry());
    REQUIRE(result.get_managers_directory() ==
      fixture.m_trading_group.get_managers_directory());
    REQUIRE(result.get_traders_directory() ==
      fixture.m_trading_group.get_traders_directory());
    REQUIRE(result.get_managers().size() == 1);
    REQUIRE(result.get_managers().front() == fixture.m_manager_account);
    REQUIRE(result.get_traders().size() == 1);
    REQUIRE(result.get_traders().front() == fixture.m_trader_account);
  }

  TEST_CASE("load_administrators") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto admin_group = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "administrators");
    auto admin_account = fixture.make_account("admin2", admin_group);
    auto result =
      fixture.m_admin_client->send_request<LoadAdministratorsService>();
    REQUIRE(result.size() == 2);
    REQUIRE(std::find(result.begin(), result.end(), fixture.m_admin_account) !=
      result.end());
    REQUIRE(
      std::find(result.begin(), result.end(), admin_account) != result.end());
  }

  TEST_CASE("load_services") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.get_root();
    auto services_group = service_locator.load_directory_entry(
      DirectoryEntry::STAR_DIRECTORY, "services");
    auto service_account1 = fixture.make_account("service1", services_group);
    auto service_account2 = fixture.make_account("service2", services_group);
    auto result = fixture.m_admin_client->send_request<LoadServicesService>();
    REQUIRE(result.size() == 2);
    REQUIRE(std::find(result.begin(), result.end(), service_account1) !=
      result.end());
    REQUIRE(std::find(result.begin(), result.end(), service_account2) !=
      result.end());
  }

  TEST_CASE("load_entitlements") {
    auto fixture = Fixture();
    auto result =
      fixture.m_trader_client->send_request<LoadEntitlementsService>();
    test_json_equality(result, fixture.m_entitlements);
  }

  TEST_CASE("load_managed_trading_groups") {
    auto fixture = Fixture();
    auto trading_group1 = fixture.make_trading_group("group1");
    auto trading_group2 = fixture.make_trading_group("group2");
    auto manager_account = fixture.make_account(
      "manager44", trading_group1.get_managers_directory());
    SUBCASE("admin") {
      auto result = fixture.m_admin_client->send_request<
        LoadManagedTradingGroupsService>(fixture.m_admin_account);
      REQUIRE(result.size() == 3);
      REQUIRE(std::find(result.begin(), result.end(),
        fixture.m_trading_group.get_entry()) != result.end());
      REQUIRE(
        std::find(result.begin(), result.end(), trading_group1.get_entry()) !=
          result.end());
      REQUIRE(
        std::find(result.begin(), result.end(), trading_group2.get_entry()) !=
          result.end());
    }
    SUBCASE("manager") {
      auto result = fixture.m_admin_client->send_request<
        LoadManagedTradingGroupsService>(manager_account);
      REQUIRE(result.size() == 1);
      REQUIRE(result.front() == trading_group1.get_entry());
    }
    SUBCASE("trader") {
      auto result = fixture.m_admin_client->send_request<
        LoadManagedTradingGroupsService>(fixture.m_trader_account);
      REQUIRE(result.empty());
    }
  }

  TEST_CASE("load_summaries_sorted_by_account_name") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto comment = Nexus::Message(0, fixture.m_admin_account,
      fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto submit = [&] (const auto& account) {
      fixture.m_admin_client->send_request<
        SubmitEntitlementModificationRequestService>(
          account, modification, ptime(), comment);
    };
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    query.set_sort_field(AccountModificationRequestQuery::SortField::ACCOUNT);
    auto load = [&] {
      return fixture.m_admin_client->send_request<
        LoadAccountModificationRequestSummariesService>(query);
    };
    SUBCASE("orders_by_name") {
      for(auto& account : {fixture.m_trader_account, fixture.m_admin_account,
          fixture.m_manager_account}) {
        submit(account);
      }
      auto summaries = load();
      REQUIRE(summaries.size() == 3);
      REQUIRE(summaries[0].m_request.get_account().m_name == "admin");
      REQUIRE(summaries[1].m_request.get_account().m_name == "manager");
      REQUIRE(summaries[2].m_request.get_account().m_name == "trader");
      query.set_snapshot_limit(SnapshotLimit::from_head(2));
      auto page = load();
      REQUIRE(page.size() == 2);
      REQUIRE(page[0].m_request.get_account().m_name == "admin");
      REQUIRE(page[1].m_request.get_account().m_name == "manager");
      query.set_anchor(AccountModificationRequestAnchor(
        page[1].m_request.get_id(), not_a_date_time,
        page[1].m_request.get_account().m_name));
      auto next = load();
      REQUIRE(next.size() == 1);
      REQUIRE(next[0].m_request.get_account().m_name == "trader");
      query.set_anchor(optional<AccountModificationRequestAnchor>());
      query.set_offset(1);
      auto skipped = load();
      REQUIRE(skipped.size() == 2);
      REQUIRE(skipped[0].m_request.get_account().m_name == "manager");
      REQUIRE(skipped[1].m_request.get_account().m_name == "trader");
      query.set_offset(0);
      query.set_snapshot_limit(SnapshotLimit::from_tail(2));
      auto tail = load();
      REQUIRE(tail.size() == 2);
      REQUIRE(tail[0].m_request.get_account().m_name == "manager");
      REQUIRE(tail[1].m_request.get_account().m_name == "trader");
      query.set_snapshot_limit(SnapshotLimit::from_head(10));
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::REQUESTER);
      auto requesters = load();
      REQUIRE(requesters.size() == 3);
      for(auto& summary : requesters) {
        REQUIRE(summary.m_request.get_submission_account().m_name == "admin");
      }
    }
    SUBCASE("ignores_case") {
      auto traders = fixture.m_trading_group.get_traders_directory();
      for(auto& name : {"Zulu", "alpha", "Bravo"}) {
        submit(fixture.make_account(name, traders));
      }
      auto summaries = load();
      REQUIRE(summaries.size() == 3);
      REQUIRE(summaries[0].m_request.get_account().m_name == "alpha");
      REQUIRE(summaries[1].m_request.get_account().m_name == "Bravo");
      REQUIRE(summaries[2].m_request.get_account().m_name == "Zulu");
    }
  }

  TEST_CASE("load_summaries_matching_a_search") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto traders = fixture.m_trading_group.get_traders_directory();
    for(auto& name : {"alpha", "beta", "gamma"}) {
      fixture.m_admin_client->send_request<
        SubmitEntitlementModificationRequestService>(
          fixture.make_account(name, traders), modification, ptime(), comment);
    }
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    query.set_search("BET");
    auto summaries = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(summaries.size() == 1);
    REQUIRE(summaries[0].m_request.get_account().m_name == "beta");
    query.set_search("a");
    auto every_name = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(every_name.size() == 3);
    query.set_search("nobody");
    auto none = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(none.empty());
  }

  TEST_CASE("load_summaries_paged_with_a_search") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      for(auto id : {1, 2, 3, 4, 5, 6}) {
        fixture.m_data_store.store(AccountModificationRequest(
          id, AccountModificationRequest::Type::ENTITLEMENTS,
          fixture.m_trader_account, fixture.m_trader_account,
          time_from_string("2024-07-04 12:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      }
    });
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_tail(3));
    query.set_search("trader");
    auto page = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(page.size() == 3);
    REQUIRE(page[0].m_request.get_id() == 4);
    REQUIRE(page[2].m_request.get_id() == 6);
    query.set_anchor(AccountModificationRequestAnchor(
      page[0].m_request.get_id(), page[0].m_request.get_timestamp(), ""));
    auto next = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(next.size() == 3);
    REQUIRE(next[0].m_request.get_id() == 1);
    REQUIRE(next[2].m_request.get_id() == 3);
  }

  TEST_CASE("load_summaries_matching_a_status") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto pending = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, modification, ptime(), comment);
    auto rejected_request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, modification, ptime(), comment);
    fixture.m_admin_client->send_request<
      RejectAccountModificationRequestService>(
        rejected_request.get_id(), comment);
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    query.set_statuses({AccountModificationRequest::Status::REJECTED});
    auto rejected = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(rejected.size() == 1);
    REQUIRE(rejected[0].m_request.get_id() == rejected_request.get_id());
    query.set_statuses({AccountModificationRequest::Status::PENDING});
    auto still_pending = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(still_pending.size() == 1);
    REQUIRE(still_pending[0].m_request.get_id() == pending.get_id());
    auto counts = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestCountsService>(query);
    REQUIRE(counts.m_rejected == 1);
    REQUIRE(counts.m_pending == 1);
    REQUIRE(counts.m_granted == 0);
  }

  TEST_CASE("load_summaries_searching_by_id_and_requester") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      for(auto id : {11, 12, 23}) {
        fixture.m_data_store.store(AccountModificationRequest(
          id, AccountModificationRequest::Type::ENTITLEMENTS,
          fixture.m_trader_account, fixture.m_manager_account,
          time_from_string("2024-07-04 12:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      }
    });
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    query.set_search("23");
    auto by_id = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(by_id.size() == 1);
    REQUIRE(by_id[0].m_request.get_id() == 23);
    query.set_search("1");
    auto by_partial_id = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(by_partial_id.size() == 2);
    REQUIRE(by_partial_id[0].m_request.get_id() == 11);
    REQUIRE(by_partial_id[1].m_request.get_id() == 12);
    query.set_search("manager");
    auto by_requester = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(by_requester.size() == 3);
  }

  TEST_CASE("load_summaries_sorted_by_date") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto effective_dates = std::vector({
      time_from_string("2024-08-03 00:00:00"),
      time_from_string("2024-08-02 00:00:00"),
      time_from_string("2024-08-01 00:00:00")});
    auto update_times = std::vector({
      time_from_string("2024-07-05 00:00:00"),
      time_from_string("2024-07-06 00:00:00"),
      time_from_string("2024-07-07 00:00:00")});
    fixture.m_data_store.with_transaction([&] {
      for(auto i = 0; i != 3; ++i) {
        fixture.m_data_store.store(AccountModificationRequest(
          i + 1, AccountModificationRequest::Type::ENTITLEMENTS,
          fixture.m_trader_account, fixture.m_trader_account,
          time_from_string("2024-07-04 12:00:00"), effective_dates[i]),
          modification);
        fixture.m_data_store.store(i + 1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::PENDING,
          fixture.m_trader_account, 0, update_times[i]));
      }
    });
    auto query = AccountModificationRequestQuery();
    query.set_index(fixture.m_trader_account);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    query.set_sort_field(
      AccountModificationRequestQuery::SortField::EFFECTIVE_DATE);
    auto by_effective_date = fixture.m_trader_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(by_effective_date.size() == 3);
    REQUIRE(by_effective_date[0].m_request.get_id() == 3);
    REQUIRE(by_effective_date[1].m_request.get_id() == 2);
    REQUIRE(by_effective_date[2].m_request.get_id() == 1);
    query.set_sort_field(
      AccountModificationRequestQuery::SortField::LAST_UPDATED);
    auto by_last_update = fixture.m_trader_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(by_last_update.size() == 3);
    REQUIRE(by_last_update[0].m_request.get_id() == 1);
    REQUIRE(by_last_update[1].m_request.get_id() == 2);
    REQUIRE(by_last_update[2].m_request.get_id() == 3);
  }

  TEST_CASE("load_summaries_sorted_by_name_anchored_from_the_tail") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto traders = fixture.m_trading_group.get_traders_directory();
    for(auto& name : {"alpha", "bravo", "charlie", "zulu"}) {
      fixture.m_admin_client->send_request<
        SubmitEntitlementModificationRequestService>(
          fixture.make_account(name, traders), modification, ptime(), comment);
    }
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_sort_field(AccountModificationRequestQuery::SortField::ACCOUNT);
    query.set_snapshot_limit(SnapshotLimit::from_tail(2));
    auto tail = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(tail.size() == 2);
    REQUIRE(tail[0].m_request.get_account().m_name == "charlie");
    REQUIRE(tail[1].m_request.get_account().m_name == "zulu");
    query.set_anchor(AccountModificationRequestAnchor(
      tail[1].m_request.get_id(), not_a_date_time,
      tail[1].m_request.get_account().m_name));
    auto previous = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(previous.size() == 2);
    REQUIRE(previous[0].m_request.get_account().m_name == "bravo");
    REQUIRE(previous[1].m_request.get_account().m_name == "charlie");
  }

  TEST_CASE("load_summaries_matching_the_filter_fields") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto parameters = RiskParameters(USD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    fixture.m_data_store.with_transaction([&] {
      fixture.m_data_store.store(AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 12:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(1, AccountModificationRequest::Update(
        AccountModificationRequest::Status::PENDING, fixture.m_trader_account,
        0, time_from_string("2024-07-05 00:00:00")));
      fixture.m_data_store.store(AccountModificationRequest(
        2, AccountModificationRequest::Type::RISK, fixture.m_manager_account,
        fixture.m_manager_account, time_from_string("2024-07-04 13:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        RiskModification(parameters));
      fixture.m_data_store.store(2, AccountModificationRequest::Update(
        AccountModificationRequest::Status::PENDING, fixture.m_manager_account,
        0, time_from_string("2024-07-09 00:00:00")));
    });
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    query.set_categories({AccountModificationRequest::Type::RISK});
    auto by_category = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(by_category.size() == 1);
    REQUIRE(by_category[0].m_request.get_id() == 2);
    query.set_categories({});
    query.set_start_date(time_from_string("2024-07-08 00:00:00"));
    auto from_start = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(from_start.size() == 1);
    REQUIRE(from_start[0].m_request.get_id() == 2);
    query.set_start_date(optional<ptime>());
    query.set_end_date(time_from_string("2024-07-06 00:00:00"));
    auto until_end = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(until_end.size() == 1);
    REQUIRE(until_end[0].m_request.get_id() == 1);
    query.set_end_date(optional<ptime>());
    query.set_excluded_account(fixture.m_manager_account);
    auto excluded = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(excluded.size() == 1);
    REQUIRE(excluded[0].m_request.get_id() == 1);
  }

  TEST_CASE("load_counts_permissions") {
    auto fixture = Fixture();
    auto trading_groups_root =
      fixture.m_service_locator_environment.get_root().load_directory_entry(
        DirectoryEntry::STAR_DIRECTORY, "trading_groups");
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      fixture.m_data_store.store(AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 12:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(AccountModificationRequest(
        2, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_manager_account, fixture.m_manager_account,
        time_from_string("2024-07-04 13:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
    });
    auto query = AccountModificationRequestQuery();
    query.set_snapshot_limit(SnapshotLimit::from_head(100));
    SUBCASE("own_account") {
      query.set_index(fixture.m_trader_account);
      auto counts = fixture.m_trader_client->send_request<
        LoadAccountModificationRequestCountsService>(query);
      REQUIRE(counts == AccountModificationRequestCounts(1, 0, 0));
    }
    SUBCASE("foreign_account_is_rejected") {
      query.set_index(fixture.m_manager_account);
      REQUIRE_THROWS(fixture.m_trader_client->send_request<
        LoadAccountModificationRequestCountsService>(query));
    }
    SUBCASE("trader_cannot_reach_root") {
      query.set_index(trading_groups_root);
      auto counts = fixture.m_trader_client->send_request<
        LoadAccountModificationRequestCountsService>(query);
      REQUIRE(counts == AccountModificationRequestCounts(0, 0, 0));
    }
    SUBCASE("manager_loads_group") {
      query.set_index(fixture.m_trading_group.get_entry());
      auto counts = fixture.m_manager_client->send_request<
        LoadAccountModificationRequestCountsService>(query);
      REQUIRE(counts == AccountModificationRequestCounts(2, 0, 0));
    }
    SUBCASE("manager_cannot_load_unmanaged_group") {
      auto other_group = fixture.make_trading_group("other_group");
      query.set_index(other_group.get_entry());
      REQUIRE_THROWS(fixture.m_manager_client->send_request<
        LoadAccountModificationRequestCountsService>(query));
    }
    SUBCASE("administrator_loads_root") {
      query.set_index(trading_groups_root);
      auto counts = fixture.m_admin_client->send_request<
        LoadAccountModificationRequestCountsService>(query);
      REQUIRE(counts == AccountModificationRequestCounts(2, 0, 0));
    }
  }

  TEST_CASE("load_counts_matching_a_search") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      fixture.m_data_store.store(AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 12:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(1, AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED, fixture.m_manager_account,
        0, time_from_string("2024-07-05 00:00:00")));
      fixture.m_data_store.store(AccountModificationRequest(
        2, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 13:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(AccountModificationRequest(
        3, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_manager_account, fixture.m_manager_account,
        time_from_string("2024-07-04 14:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(3, AccountModificationRequest::Update(
        AccountModificationRequest::Status::REJECTED, fixture.m_manager_account,
        0, time_from_string("2024-07-06 00:00:00")));
    });
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(100));
    auto total = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestCountsService>(query);
    REQUIRE(total == AccountModificationRequestCounts(1, 1, 1));
    query.set_search("trader");
    auto searched = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestCountsService>(query);
    REQUIRE(searched == AccountModificationRequestCounts(1, 1, 0));
  }

  TEST_CASE("load_counts_matching_an_expression_filter") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      fixture.m_data_store.store(AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 12:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(AccountModificationRequest(
        2, AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_manager_account, fixture.m_manager_account,
        time_from_string("2024-07-04 13:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
    });
    auto root = fixture.m_admin_client->send_request<
      LoadTradingGroupsRootEntryService>();
    auto query = AccountModificationRequestQuery();
    query.set_index(root);
    query.set_snapshot_limit(SnapshotLimit::from_head(100));
    auto accessor = AccountModificationRequestAccessor::from_parameter(0);
    query.set_filter(accessor.get_account() != ConstantExpression(
      static_cast<int>(fixture.m_manager_account.m_id)));
    auto counts = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestCountsService>(query);
    REQUIRE(counts == AccountModificationRequestCounts(1, 0, 0));
    auto summaries = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(summaries.size() == 1);
    REQUIRE(summaries[0].m_request.get_id() == 1);
  }

  TEST_CASE("summary_counts_comments") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    auto comment = Nexus::Message(0, fixture.m_trader_account,
      fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, modification, ptime(), comment);
    for(auto& text : {"first", "second"}) {
      auto message = Nexus::Message(0, fixture.m_trader_account,
        fixture.m_time_client.get_time(),
        {Nexus::Message::Body::make_plain_text(text)});
      fixture.m_trader_client->send_request<
        SendAccountModificationRequestMessageService>(
          request.get_id(), message);
    }
    auto query = AccountModificationRequestQuery();
    query.set_index(fixture.m_trader_account);
    query.set_snapshot_limit(SnapshotLimit::from_head(10));
    auto summaries = fixture.m_trader_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(summaries.size() == 1);
    REQUIRE(summaries[0].m_comment_count == 3);
  }

  TEST_CASE("submit_and_load_entitlement_modification") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    SUBCASE("admin") {
      auto request = fixture.m_admin_client->send_request<
        SubmitEntitlementModificationRequestService>(
          fixture.m_trader_account, modification, ptime(), comment);
      REQUIRE(request.get_id() == 1);
      REQUIRE(request.get_account() == fixture.m_trader_account);
      REQUIRE(request.get_submission_account() == fixture.m_admin_account);
      REQUIRE(request.get_type() ==
        AccountModificationRequest::Type::ENTITLEMENTS);
      auto loaded_request = fixture.m_admin_client->send_request<
        LoadAccountModificationRequestService>(request.get_id());
      REQUIRE(loaded_request.get_id() == request.get_id());
      REQUIRE(loaded_request.get_account() == request.get_account());
      REQUIRE(loaded_request.get_submission_account() ==
        request.get_submission_account());
      REQUIRE(loaded_request.get_timestamp() == request.get_timestamp());
      REQUIRE(loaded_request.get_type() == request.get_type());
      auto loaded_modification = fixture.m_admin_client->send_request<
        LoadEntitlementModificationService>(request.get_id());
      REQUIRE(loaded_modification.get_entitlements().size() ==
        modification.get_entitlements().size());
      for(auto& entitlement : modification.get_entitlements()) {
        REQUIRE(std::find(loaded_modification.get_entitlements().begin(),
          loaded_modification.get_entitlements().end(), entitlement) !=
          loaded_modification.get_entitlements().end());
      }
    }
    SUBCASE("trader") {
      auto request = fixture.m_trader_client->send_request<
        SubmitEntitlementModificationRequestService>(
          DirectoryEntry(), modification, ptime(), comment);
      REQUIRE(request.get_account() == fixture.m_trader_account);
      REQUIRE(request.get_submission_account() == fixture.m_trader_account);
      auto initial_status = fixture.m_trader_client->send_request<
        LoadAccountModificationRequestStatusService>(request.get_id());
      REQUIRE(
        initial_status.m_status == AccountModificationRequest::Status::PENDING);
      REQUIRE(initial_status.m_account == fixture.m_trader_account);
      auto review_comment = Nexus::Message(
        0, fixture.m_manager_account, fixture.m_time_client.get_time(),
          {Nexus::Message::Body::make_plain_text("Reviewed by manager")});
      auto review_update = fixture.m_manager_client->send_request<
        ApproveAccountModificationRequestService>(
          request.get_id(), ptime(), review_comment);
      REQUIRE(
        review_update.m_status == AccountModificationRequest::Status::REVIEWED);
      auto review_entitlements = fixture.m_manager_client->send_request<
        LoadAccountEntitlementsService>(fixture.m_trader_account);
      REQUIRE(review_entitlements.empty());
      SUBCASE("approve") {
        auto comment = Nexus::Message(
          0, fixture.m_admin_account, fixture.m_time_client.get_time(),
            {Nexus::Message::Body::make_plain_text("Approved by admin")});
        auto update = fixture.m_admin_client->send_request<
          ApproveAccountModificationRequestService>(
            request.get_id(), ptime(), comment);
        REQUIRE(
          update.m_status == AccountModificationRequest::Status::GRANTED);
        REQUIRE(update.m_account == fixture.m_admin_account);
        auto status = fixture.m_trader_client->send_request<
          LoadAccountModificationRequestStatusService>(request.get_id());
        REQUIRE(
          status.m_status == AccountModificationRequest::Status::GRANTED);
        REQUIRE(status.m_account == fixture.m_admin_account);
        auto entitlements = fixture.m_manager_client->send_request<
          LoadAccountEntitlementsService>(fixture.m_trader_account);
        REQUIRE(
          entitlements.size() == fixture.m_entitlements.get_entries().size());
        for(auto& entitlement : fixture.m_entitlements.get_entries()) {
          REQUIRE(std::find(entitlements.begin(), entitlements.end(),
            entitlement.m_group_entry) != entitlements.end());
        }
      }
      SUBCASE("reject") {
        auto comment = Nexus::Message(
          0, fixture.m_admin_account, fixture.m_time_client.get_time(),
            {Nexus::Message::Body::make_plain_text("Rejected by admin")});
        auto update = fixture.m_admin_client->send_request<
          RejectAccountModificationRequestService>(request.get_id(), comment);
        REQUIRE(
          update.m_status == AccountModificationRequest::Status::REJECTED);
        REQUIRE(update.m_account == fixture.m_admin_account);
        auto status = fixture.m_trader_client->send_request<
          LoadAccountModificationRequestStatusService>(request.get_id());
        REQUIRE(
          status.m_status == AccountModificationRequest::Status::REJECTED);
        REQUIRE(status.m_account == fixture.m_admin_account);
        auto entitlements = fixture.m_manager_client->send_request<
          LoadAccountEntitlementsService>(fixture.m_trader_account);
        REQUIRE(entitlements.empty());
      }
    }
  }

  TEST_CASE("entitlement_modification_granted_notification") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto review_comment = Nexus::Message(
      0, fixture.m_manager_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("reviewed")});
    fixture.m_manager_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), review_comment);
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("approved")});
    fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), approve_comment);
    auto notifications = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::from_tail(1), Notification::ReadState::ALL);
    REQUIRE(notifications.size() == 1);
    REQUIRE(notifications[0].m_account == fixture.m_trader_account);
    REQUIRE(notifications[0].m_description ==
      "Entitlements have been updated.");
    REQUIRE(notifications[0].m_category ==
      Notification::Category::ACCOUNT_MODIFICATION);
  }

  TEST_CASE("entitlement_modification_rejected_notification") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto reject_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("rejected")});
    fixture.m_admin_client->send_request<
      RejectAccountModificationRequestService>(
        request.get_id(), reject_comment);
    auto notifications = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::from_tail(1), Notification::ReadState::ALL);
    REQUIRE(notifications.size() == 1);
    REQUIRE(notifications[0].m_account == fixture.m_trader_account);
    REQUIRE(notifications[0].m_description ==
      "Entitlement modification request has been rejected.");
    REQUIRE(notifications[0].m_category ==
      Notification::Category::ACCOUNT_MODIFICATION);
  }

  TEST_CASE("approve_entitlements_with_future_effective_date") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved with future date")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), future_date, approve_comment);
    REQUIRE(
      update.m_status == AccountModificationRequest::Status::SCHEDULED);
    REQUIRE(update.m_account == fixture.m_admin_account);
    auto status = fixture.m_trader_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(
      status.m_status == AccountModificationRequest::Status::SCHEDULED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.empty());
  }

  TEST_CASE("approve_entitlements_with_past_effective_date") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto past_date = time_from_string("2024-06-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved with past date")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), past_date, approve_comment);
    REQUIRE(update.m_status == AccountModificationRequest::Status::GRANTED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.size() ==
      fixture.m_entitlements.get_entries().size());
  }

  TEST_CASE("approve_entitlements_with_not_a_date_time") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved immediately")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), approve_comment);
    REQUIRE(update.m_status == AccountModificationRequest::Status::GRANTED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.size() ==
      fixture.m_entitlements.get_entries().size());
  }

  TEST_CASE("approve_entitlements_with_request_future_effective_date") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, future_date, comment);
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), approve_comment);
    REQUIRE(
      update.m_status == AccountModificationRequest::Status::SCHEDULED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.empty());
  }

  TEST_CASE("approve_risk_with_future_effective_date") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitRiskModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved with future date")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), future_date, approve_comment);
    REQUIRE(update.m_status == AccountModificationRequest::Status::SCHEDULED);
  }

  TEST_CASE("approve_risk_with_past_effective_date") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitRiskModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto past_date = time_from_string("2024-06-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved with past date")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), past_date, approve_comment);
    REQUIRE(update.m_status == AccountModificationRequest::Status::GRANTED);
  }

  TEST_CASE("grant_scheduled_entitlements_on_startup") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), future_date, approve_comment);
    REQUIRE(
      update.m_status == AccountModificationRequest::Status::SCHEDULED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.empty());
    fixture.m_admin_client.reset();
    fixture.m_manager_client.reset();
    fixture.m_trader_client.reset();
    fixture.m_container.reset();
    fixture.m_time_client.set(time_from_string("2024-08-02 12:00:00"));
    fixture.m_server_connection =
      std::make_shared<LocalServerConnection>();
    fixture.m_container.emplace(init(*fixture.m_servlet_service_locator_client,
      init(&fixture.m_service_locator_environment.get_root(),
        fixture.m_entitlements, &fixture.m_data_store,
        &fixture.m_time_client, &fixture.m_timer)),
      fixture.m_server_connection,
      factory<std::unique_ptr<TriggerTimer>>());
    std::tie(std::ignore, fixture.m_admin_client) =
      fixture.make_client("admin");
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::GRANTED);
    loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(
      loaded_entitlements.size() == fixture.m_entitlements.get_entries().size());
  }

  TEST_CASE("scheduled_not_granted_before_effective_date") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), future_date, approve_comment);
    REQUIRE(
      update.m_status == AccountModificationRequest::Status::SCHEDULED);
    fixture.m_admin_client.reset();
    fixture.m_manager_client.reset();
    fixture.m_trader_client.reset();
    fixture.m_container.reset();
    fixture.m_time_client.set(time_from_string("2024-07-15 12:00:00"));
    fixture.m_server_connection =
      std::make_shared<LocalServerConnection>();
    fixture.m_container.emplace(init(*fixture.m_servlet_service_locator_client,
      init(&fixture.m_service_locator_environment.get_root(),
        fixture.m_entitlements, &fixture.m_data_store,
        &fixture.m_time_client, &fixture.m_timer)),
      fixture.m_server_connection,
      factory<std::unique_ptr<TriggerTimer>>());
    std::tie(std::ignore, fixture.m_admin_client) =
      fixture.make_client("admin");
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(
      status.m_status == AccountModificationRequest::Status::SCHEDULED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.empty());
  }

  TEST_CASE("submit_and_load_risk_modification") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
        {Nexus::Message::Body::make_plain_text("test comment")});
    SUBCASE("admin") {
      auto request = fixture.m_admin_client->send_request<
        SubmitRiskModificationRequestService>(
          fixture.m_trader_account, modification, ptime(), comment);
      REQUIRE(request.get_id() == 1);
      REQUIRE(request.get_account() == fixture.m_trader_account);
      REQUIRE(request.get_submission_account() == fixture.m_admin_account);
      REQUIRE(request.get_type() == AccountModificationRequest::Type::RISK);
      auto loaded_request = fixture.m_admin_client->send_request<
        LoadAccountModificationRequestService>(request.get_id());
      REQUIRE(loaded_request.get_id() == request.get_id());
      REQUIRE(loaded_request.get_account() == request.get_account());
      REQUIRE(loaded_request.get_submission_account() ==
        request.get_submission_account());
      REQUIRE(loaded_request.get_timestamp() == request.get_timestamp());
      REQUIRE(loaded_request.get_type() == request.get_type());
      auto loaded_modification = fixture.m_admin_client->send_request<
        LoadRiskModificationService>(request.get_id());
      REQUIRE(loaded_modification.get_parameters() ==
        modification.get_parameters());
    }
    SUBCASE("trader") {
      auto request = fixture.m_trader_client->send_request<
        SubmitRiskModificationRequestService>(
          DirectoryEntry(), modification, ptime(), comment);
      REQUIRE(request.get_account() == fixture.m_trader_account);
      REQUIRE(request.get_submission_account() == fixture.m_trader_account);
      auto initial_status = fixture.m_trader_client->send_request<
        LoadAccountModificationRequestStatusService>(request.get_id());
      REQUIRE(
        initial_status.m_status == AccountModificationRequest::Status::PENDING);
      REQUIRE(initial_status.m_account == fixture.m_trader_account);
      auto review_comment = Nexus::Message(
        0, fixture.m_manager_account, fixture.m_time_client.get_time(),
          {Nexus::Message::Body::make_plain_text("Rejected by manager.")});
      auto review_update = fixture.m_manager_client->send_request<
        RejectAccountModificationRequestService>(
        request.get_id(), review_comment);
      REQUIRE(
        review_update.m_status == AccountModificationRequest::Status::REJECTED);
    }
  }

  TEST_CASE("risk_modification_granted_notification") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test")});
    auto request = fixture.m_trader_client->send_request<
      SubmitRiskModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto review_comment = Nexus::Message(
      0, fixture.m_manager_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("reviewed")});
    fixture.m_manager_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), review_comment);
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("approved")});
    fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), approve_comment);
    auto notifications = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::from_tail(1), Notification::ReadState::ALL);
    REQUIRE(notifications.size() == 1);
    REQUIRE(notifications[0].m_account == fixture.m_trader_account);
    REQUIRE(notifications[0].m_description ==
      "Risk parameters have been updated.");
    REQUIRE(notifications[0].m_category ==
      Notification::Category::ACCOUNT_MODIFICATION);
  }

  TEST_CASE("risk_modification_rejected_notification") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test")});
    auto request = fixture.m_trader_client->send_request<
      SubmitRiskModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto reject_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("rejected")});
    fixture.m_admin_client->send_request<
      RejectAccountModificationRequestService>(
        request.get_id(), reject_comment);
    auto notifications = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::from_tail(1), Notification::ReadState::ALL);
    REQUIRE(notifications.size() == 1);
    REQUIRE(notifications[0].m_account == fixture.m_trader_account);
    REQUIRE(notifications[0].m_description ==
      "Risk modification request has been rejected.");
    REQUIRE(notifications[0].m_category ==
      Notification::Category::ACCOUNT_MODIFICATION);
  }

  TEST_CASE("admin_submit_entitlements_with_future_effective_date") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_admin_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, modification, future_date, comment);
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(
      status.m_status == AccountModificationRequest::Status::SCHEDULED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.empty());
  }

  TEST_CASE("admin_submit_entitlements_with_past_effective_date") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto past_date = time_from_string("2024-06-01 00:00:00");
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_admin_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, modification, past_date, comment);
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::GRANTED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(
      loaded_entitlements.size() == fixture.m_entitlements.get_entries().size());
  }

  TEST_CASE("admin_submit_entitlements_with_not_a_date_time") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_admin_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, modification, ptime(), comment);
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::GRANTED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(
      loaded_entitlements.size() == fixture.m_entitlements.get_entries().size());
  }

  TEST_CASE("admin_submit_risk_with_future_effective_date") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_admin_client->send_request<
      SubmitRiskModificationRequestService>(
        fixture.m_trader_account, modification, future_date, comment);
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(
      status.m_status == AccountModificationRequest::Status::SCHEDULED);
  }

  TEST_CASE("admin_submit_risk_with_past_effective_date") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto past_date = time_from_string("2024-06-01 00:00:00");
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_admin_client->send_request<
      SubmitRiskModificationRequestService>(
        fixture.m_trader_account, modification, past_date, comment);
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::GRANTED);
  }

  TEST_CASE("admin_submit_risk_with_not_a_date_time") {
    auto fixture = Fixture();
    auto parameters = RiskParameters(
      AUD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(1));
    auto modification = RiskModification(parameters);
    auto comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_admin_client->send_request<
      SubmitRiskModificationRequestService>(
        fixture.m_trader_account, modification, ptime(), comment);
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::GRANTED);
  }

  TEST_CASE("send_notification") {
    auto fixture = Fixture();
    auto notification = fixture.m_admin_client->send_request<
      SendNotificationService>(fixture.m_trader_account,
        "Your request has been approved.", "{\"request_id\":42}",
        Notification::Category::ACCOUNT_MODIFICATION);
    REQUIRE(!notification.m_id.empty());
    REQUIRE(notification.m_account == fixture.m_trader_account);
    REQUIRE(notification.m_description == "Your request has been approved.");
    REQUIRE(notification.m_data == "{\"request_id\":42}");
    REQUIRE(notification.m_category ==
      Notification::Category::ACCOUNT_MODIFICATION);
    REQUIRE(notification.m_timestamp == fixture.m_time_client.get_time());
    REQUIRE(!notification.m_is_read);
  }

  TEST_CASE("modification_requests_are_scoped_to_a_readable_account") {
    auto fixture = Fixture();
    auto outsider_group = fixture.make_trading_group("outsider_group");
    fixture.make_account("outsider", outsider_group.get_traders_directory());
    auto [outsider_account, outsider_client] = fixture.make_client("outsider");
    auto comment =
      Nexus::Message(0, outsider_account, fixture.m_time_client.get_time(),
        {Nexus::Message::Body::make_plain_text("test comment")});
    SUBCASE("submitting_for_another_group") {
      REQUIRE_THROWS_AS(outsider_client->send_request<
        SubmitEntitlementModificationRequestService>(fixture.m_trader_account,
          EntitlementModification(), ptime(), comment), ServiceRequestException);
      REQUIRE_THROWS_AS(outsider_client->send_request<
        SubmitRiskModificationRequestService>(fixture.m_trader_account,
          RiskModification(), ptime(), comment), ServiceRequestException);
    }
    SUBCASE("submitting_for_oneself") {
      REQUIRE_NOTHROW(outsider_client->send_request<
        SubmitEntitlementModificationRequestService>(outsider_account,
          EntitlementModification(), ptime(), comment));
    }
    SUBCASE("reading_and_commenting_on_another_group") {
      auto request = fixture.m_trader_client->send_request<
        SubmitEntitlementModificationRequestService>(fixture.m_trader_account,
          EntitlementModification(), ptime(), comment);
      auto ids = fixture.m_trader_client->send_request<
        LoadMessageIdsService>(request.get_id());
      REQUIRE(ids.size() == 1);
      REQUIRE_THROWS_AS(outsider_client->send_request<LoadMessageService>(
        request.get_id(), ids[0]), ServiceRequestException);
      REQUIRE_THROWS_AS(outsider_client->send_request<
        SendAccountModificationRequestMessageService>(
          request.get_id(), comment), ServiceRequestException);
      REQUIRE_NOTHROW(fixture.m_trader_client->send_request<
        LoadMessageService>(request.get_id(), ids[0]));
    }
  }

  TEST_CASE("loading_a_message_from_an_unrelated_request") {
    auto fixture = Fixture();
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto first = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(fixture.m_trader_account,
        EntitlementModification(), ptime(), comment);
    auto second = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        fixture.m_trader_account, EntitlementModification(), ptime(), comment);
    auto ids = fixture.m_trader_client->send_request<LoadMessageIdsService>(
      second.get_id());
    REQUIRE(ids.size() == 1);
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      LoadMessageService>(first.get_id(), ids[0]), ServiceRequestException);
  }

  TEST_CASE("approving_can_not_lower_a_request_status") {
    auto fixture = Fixture();
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(fixture.m_trader_account,
        EntitlementModification(), ptime(), comment);
    auto reviewed = fixture.m_manager_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), comment);
    REQUIRE(reviewed.m_status == AccountModificationRequest::Status::REVIEWED);
    auto messages = fixture.m_data_store.load_message_ids(request.get_id());
    auto effective_date = fixture.m_data_store.
      load_account_modification_request(request.get_id()).get_effective_date();
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), time_from_string("2030-01-01 00:00:00"), comment),
      ServiceRequestException);
    auto status = fixture.m_manager_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::REVIEWED);
    REQUIRE(
      fixture.m_data_store.load_message_ids(request.get_id()) == messages);
    REQUIRE(fixture.m_data_store.load_account_modification_request(
      request.get_id()).get_effective_date() == effective_date);
  }

  TEST_CASE("approving_reads_a_concurrently_stored_effective_date") {
    auto fixture = Fixture<InterceptingDataStore>();
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(fixture.m_trader_account,
        EntitlementModification(), ptime(), comment);
    auto scheduled_date = time_from_string("2024-08-01 00:00:00");
    fixture.m_data_store.m_on_load_status = [&] {
      fixture.m_data_store.store_effective_date(
        request.get_id(), scheduled_date);
    };
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), ptime(), comment);
    REQUIRE(update.m_status == AccountModificationRequest::Status::SCHEDULED);
    REQUIRE(fixture.m_data_store.load_account_modification_request(
      request.get_id()).get_effective_date() == scheduled_date);
  }

  TEST_CASE("send_notification_insufficient_permissions") {
    auto fixture = Fixture();
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      SendNotificationService>(fixture.m_trader_account, "test", "",
        Notification::Category::REPORT), ServiceRequestException);
  }

  TEST_CASE("send_notification_stored_in_data_store") {
    auto fixture = Fixture();
    auto notification = fixture.m_admin_client->send_request<
      SendNotificationService>(fixture.m_trader_account, "Stored notification.",
        "", Notification::Category::REPORT);
    auto loaded = fixture.m_data_store.with_transaction([&] {
      return fixture.m_data_store.load_notifications(
        fixture.m_trader_account, "", SnapshotLimit::UNLIMITED,
        Notification::ReadState::ALL);
    });
    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded[0].m_id == notification.m_id);
    REQUIRE(loaded[0].m_description == "Stored notification.");
  }

  TEST_CASE("monitor_notifications_returns_last_id") {
    auto fixture = Fixture();
    auto notification =
      fixture.m_admin_client->send_request<SendNotificationService>(
        fixture.m_trader_account, "First.", "", Notification::Category::REPORT);
    auto last_id = fixture.m_trader_client->send_request<
      MonitorNotificationsService>(fixture.m_trader_account);
    REQUIRE(last_id == notification.m_id);
  }

  TEST_CASE("monitor_notifications_empty") {
    auto fixture = Fixture();
    auto last_id = fixture.m_trader_client->send_request<
      MonitorNotificationsService>(fixture.m_trader_account);
    REQUIRE(last_id.empty());
  }

  TEST_CASE("load_notifications") {
    auto fixture = Fixture();
    fixture.m_admin_client->send_request<SendNotificationService>(
      fixture.m_trader_account, "First.", "",
      Notification::Category::ACCOUNT_MODIFICATION);
    fixture.m_admin_client->send_request<SendNotificationService>(
      fixture.m_trader_account, "Second.", "", Notification::Category::REPORT);
    auto notifications = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
    REQUIRE(notifications.size() == 2);
    REQUIRE(notifications[0].m_description == "First.");
    REQUIRE(notifications[1].m_description == "Second.");
  }

  TEST_CASE("load_notifications_unread_filter") {
    auto fixture = Fixture();
    auto notification =
      fixture.m_admin_client->send_request<SendNotificationService>(
        fixture.m_trader_account, "Unread.", "",
        Notification::Category::REPORT);
    fixture.m_admin_client->send_request<SendNotificationService>(
      fixture.m_trader_account, "Also unread.", "",
      Notification::Category::REPORT);
    auto unread =
      fixture.m_trader_client->send_request<LoadNotificationsService>(
        fixture.m_trader_account, "", SnapshotLimit::UNLIMITED,
        Notification::ReadState::UNREAD);
    REQUIRE(unread.size() == 2);
  }

  TEST_CASE("load_notifications_tail_limit") {
    auto fixture = Fixture();
    fixture.m_admin_client->send_request<SendNotificationService>(
      fixture.m_trader_account, "First.", "", Notification::Category::REPORT);
    fixture.m_admin_client->send_request<SendNotificationService>(
      fixture.m_trader_account, "Second.", "", Notification::Category::REPORT);
    fixture.m_admin_client->send_request<SendNotificationService>(
      fixture.m_trader_account, "Third.", "", Notification::Category::REPORT);
    auto tail = fixture.m_trader_client->send_request<LoadNotificationsService>(
      fixture.m_trader_account, "", SnapshotLimit::from_tail(2),
      Notification::ReadState::ALL);
    REQUIRE(tail.size() == 2);
    REQUIRE(tail[0].m_description == "Second.");
    REQUIRE(tail[1].m_description == "Third.");
  }

  TEST_CASE("load_notifications_insufficient_permissions") {
    auto fixture = Fixture();
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_admin_account, "",
        SnapshotLimit::UNLIMITED, Notification::ReadState::ALL),
      ServiceRequestException);
  }

  TEST_CASE("mark_notification_as_read_own") {
    auto fixture = Fixture();
    auto notification =
      fixture.m_admin_client->send_request<SendNotificationService>(
        fixture.m_trader_account, "To be read.", "",
        Notification::Category::REPORT);
    REQUIRE_NOTHROW(fixture.m_trader_client->send_request<
      MarkNotificationAsReadService>(notification.m_id));
    auto loaded = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::UNLIMITED, Notification::ReadState::READ);
    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded[0].m_id == notification.m_id);
    REQUIRE(loaded[0].m_is_read);
  }

  TEST_CASE("mark_notification_as_read_admin") {
    auto fixture = Fixture();
    auto notification = fixture.m_admin_client->send_request<
      SendNotificationService>(fixture.m_trader_account, "Admin marks.", "",
        Notification::Category::REPORT);
    REQUIRE_NOTHROW(fixture.m_admin_client->send_request<
      MarkNotificationAsReadService>(notification.m_id));
    auto loaded = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::UNLIMITED, Notification::ReadState::READ);
    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded[0].m_is_read);
  }

  TEST_CASE("mark_notification_as_read_insufficient_permissions") {
    auto fixture = Fixture();
    auto notification = fixture.m_admin_client->send_request<
      SendNotificationService>(fixture.m_admin_account, "Admin only.", "",
        Notification::Category::REPORT);
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      MarkNotificationAsReadService>(notification.m_id),
      ServiceRequestException);
  }

  TEST_CASE("mark_notification_as_unread_own") {
    auto fixture = Fixture();
    auto notification =
      fixture.m_admin_client->send_request<SendNotificationService>(
        fixture.m_trader_account, "To be unread.", "",
        Notification::Category::REPORT);
    fixture.m_trader_client->send_request<
      MarkNotificationAsReadService>(notification.m_id);
    REQUIRE_NOTHROW(fixture.m_trader_client->send_request<
      MarkNotificationAsUnreadService>(notification.m_id));
    auto loaded = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::UNLIMITED, Notification::ReadState::UNREAD);
    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded[0].m_id == notification.m_id);
    REQUIRE(!loaded[0].m_is_read);
  }

  TEST_CASE("mark_notification_as_unread_admin") {
    auto fixture = Fixture();
    auto notification = fixture.m_admin_client->send_request<
      SendNotificationService>(fixture.m_trader_account, "Admin unmarks.", "",
        Notification::Category::REPORT);
    fixture.m_trader_client->send_request<
      MarkNotificationAsReadService>(notification.m_id);
    REQUIRE_NOTHROW(fixture.m_admin_client->send_request<
      MarkNotificationAsUnreadService>(notification.m_id));
    auto loaded = fixture.m_trader_client->send_request<
      LoadNotificationsService>(fixture.m_trader_account, "",
        SnapshotLimit::UNLIMITED, Notification::ReadState::UNREAD);
    REQUIRE(loaded.size() == 1);
    REQUIRE(!loaded[0].m_is_read);
  }

  TEST_CASE("mark_notification_as_unread_insufficient_permissions") {
    auto fixture = Fixture();
    auto notification = fixture.m_admin_client->send_request<
      SendNotificationService>(fixture.m_admin_account, "Admin only.", "",
        Notification::Category::REPORT);
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      MarkNotificationAsUnreadService>(notification.m_id),
      ServiceRequestException);
  }

  TEST_CASE("monitor_notifications_insufficient_permissions") {
    auto fixture = Fixture();
    REQUIRE_THROWS_AS(fixture.m_trader_client->send_request<
      MonitorNotificationsService>(fixture.m_admin_account),
      ServiceRequestException);
  }

  TEST_CASE("grant_scheduled_entitlements_on_timer") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(
      0, fixture.m_admin_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved")});
    auto update = fixture.m_admin_client->send_request<
      ApproveAccountModificationRequestService>(
        request.get_id(), future_date, approve_comment);
    REQUIRE(update.m_status == AccountModificationRequest::Status::SCHEDULED);
    auto loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.empty());
    fixture.m_time_client.set(time_from_string("2024-08-02 12:00:00"));
    fixture.m_timer.trigger();
    flush_pending_routines();
    auto status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(request.get_id());
    REQUIRE(status.m_status == AccountModificationRequest::Status::GRANTED);
    loaded_entitlements = fixture.m_admin_client->send_request<
      LoadAccountEntitlementsService>(fixture.m_trader_account);
    REQUIRE(loaded_entitlements.size() ==
      fixture.m_entitlements.get_entries().size());
  }

  TEST_CASE("submitting_without_an_effective_date_resolves_it") {
    auto fixture = Fixture();
    auto comment = Nexus::Message(0, fixture.m_trader_account,
      fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto entitlements = EntitlementModification();
    auto entitlement_request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), entitlements, ptime(), comment);
    REQUIRE(entitlement_request.get_effective_date() ==
      entitlement_request.get_timestamp());
    auto risk = RiskModification(RiskParameters());
    auto risk_request = fixture.m_trader_client->send_request<
      SubmitRiskModificationRequestService>(
        DirectoryEntry(), risk, ptime(), comment);
    REQUIRE(risk_request.get_effective_date() ==
      risk_request.get_timestamp());
    auto scheduled_date = time_from_string("2024-08-01 00:00:00");
    auto scheduled_request = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), entitlements, scheduled_date, comment);
    REQUIRE(scheduled_request.get_effective_date() == scheduled_date);
    for(auto id : {entitlement_request.get_id(), risk_request.get_id(),
        scheduled_request.get_id()}) {
      auto stored = fixture.m_data_store.load_account_modification_request(id);
      REQUIRE(stored.get_effective_date() != ptime());
    }
  }

  TEST_CASE("scheduled_grant_respects_a_concurrent_reschedule") {
    auto fixture = Fixture<InterceptingDataStore>();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(
      0, fixture.m_trader_account, fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto first = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto second = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(0, fixture.m_admin_account,
      fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved")});
    for(auto id : {first.get_id(), second.get_id()}) {
      auto update = fixture.m_admin_client->send_request<
        ApproveAccountModificationRequestService>(
          id, future_date, approve_comment);
      REQUIRE(update.m_status == AccountModificationRequest::Status::SCHEDULED);
    }
    auto rescheduled_date = time_from_string("2024-09-01 00:00:00");
    fixture.m_data_store.m_on_load_entitlement_modification = [&] {
      fixture.m_data_store.store_effective_date(
        second.get_id(), rescheduled_date);
    };
    fixture.m_time_client.set(time_from_string("2024-08-02 12:00:00"));
    fixture.m_timer.trigger();
    flush_pending_routines();
    auto first_status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(first.get_id());
    REQUIRE(
      first_status.m_status == AccountModificationRequest::Status::GRANTED);
    auto second_status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(second.get_id());
    REQUIRE(
      second_status.m_status == AccountModificationRequest::Status::SCHEDULED);
    REQUIRE(fixture.m_data_store.load_account_modification_request(
      second.get_id()).get_effective_date() == rescheduled_date);
  }

  TEST_CASE("scheduled_grant_respects_a_concurrent_rejection") {
    auto fixture = Fixture<InterceptingDataStore>();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message(0, fixture.m_trader_account,
      fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("test comment")});
    auto first = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto second = fixture.m_trader_client->send_request<
      SubmitEntitlementModificationRequestService>(
        DirectoryEntry(), modification, ptime(), comment);
    auto future_date = time_from_string("2024-08-01 00:00:00");
    auto approve_comment = Nexus::Message(0, fixture.m_admin_account,
      fixture.m_time_client.get_time(),
      {Nexus::Message::Body::make_plain_text("Approved")});
    for(auto id : {first.get_id(), second.get_id()}) {
      auto update = fixture.m_admin_client->send_request<
        ApproveAccountModificationRequestService>(
          id, future_date, approve_comment);
      REQUIRE(update.m_status == AccountModificationRequest::Status::SCHEDULED);
    }
    auto rejection_time = time_from_string("2024-08-02 12:00:00");
    fixture.m_data_store.m_on_load_entitlement_modification = [&] {
      auto status = fixture.m_data_store.
        load_account_modification_request_status(second.get_id());
      status.m_status = AccountModificationRequest::Status::REJECTED;
      ++status.m_sequence_number;
      status.m_timestamp = rejection_time;
      fixture.m_data_store.store(second.get_id(), status);
    };
    fixture.m_time_client.set(rejection_time);
    fixture.m_timer.trigger();
    flush_pending_routines();
    auto first_status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(first.get_id());
    REQUIRE(
      first_status.m_status == AccountModificationRequest::Status::GRANTED);
    auto second_status = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestStatusService>(second.get_id());
    REQUIRE(
      second_status.m_status == AccountModificationRequest::Status::REJECTED);
    auto updates = fixture.m_data_store.
      load_account_modification_request_updates(second.get_id());
    auto sequence_numbers = std::vector<int>();
    for(auto& update : updates) {
      sequence_numbers.push_back(update.m_sequence_number);
    }
    std::ranges::sort(sequence_numbers);
    REQUIRE(std::ranges::adjacent_find(sequence_numbers) ==
      sequence_numbers.end());
  }

  TEST_CASE("query_accounts_by_username") {
    auto fixture = Fixture();
    auto results = fixture.m_admin_client->send_request<
      QueryAccountsService>(std::string("tra"));
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].m_account == fixture.m_trader_account);
  }

  TEST_CASE("query_accounts_by_name") {
    auto fixture = Fixture();
    auto identity = AccountIdentity();
    identity.m_first_name = "John";
    identity.m_last_name = "Smith";
    fixture.m_data_store.store(fixture.m_trader_account, identity);
    SUBCASE("matches_a_first_name_prefix") {
      auto results = fixture.m_admin_client->send_request<QueryAccountsService>(
        std::string("joh"));
      REQUIRE(results.size() == 1);
      REQUIRE(results[0].m_account == fixture.m_trader_account);
      REQUIRE(results[0].m_name == "John Smith");
    }
    SUBCASE("ignores_case") {
      auto results = fixture.m_admin_client->send_request<QueryAccountsService>(
        std::string("JOHN"));
      REQUIRE(results.size() == 1);
      REQUIRE(results[0].m_account == fixture.m_trader_account);
    }
    SUBCASE("does_not_match_an_interior_substring") {
      auto results = fixture.m_admin_client->send_request<QueryAccountsService>(
        std::string("ohn"));
      REQUIRE(results.empty());
    }
    SUBCASE("does_not_match_an_unrelated_query") {
      auto results = fixture.m_admin_client->send_request<QueryAccountsService>(
        std::string("xyz"));
      REQUIRE(results.empty());
    }
  }

  TEST_CASE("query_accounts_by_multi_token_name") {
    auto fixture = Fixture();
    auto identity = AccountIdentity();
    identity.m_first_name = "John";
    identity.m_last_name = "Van Der Berg";
    fixture.m_data_store.store(fixture.m_trader_account, identity);
    SUBCASE("matches_the_first_token") {
      auto results = fixture.m_admin_client->send_request<QueryAccountsService>(
        std::string("van"));
      REQUIRE(results.size() == 1);
      REQUIRE(results[0].m_account == fixture.m_trader_account);
      REQUIRE(results[0].m_name == "John Van Der Berg");
    }
    SUBCASE("matches_a_middle_token") {
      auto results = fixture.m_admin_client->send_request<QueryAccountsService>(
        std::string("der"));
      REQUIRE(results.size() == 1);
      REQUIRE(results[0].m_account == fixture.m_trader_account);
    }
  }

  TEST_CASE("query_accounts_empty_query") {
    auto fixture = Fixture();
    auto results = fixture.m_admin_client->send_request<
      QueryAccountsService>(std::string(""));
    REQUIRE(results.size() == 3);
  }

  TEST_CASE("query_accounts_manager_permissions") {
    auto fixture = Fixture();
    SUBCASE("sees_managed_accounts") {
      auto results = fixture.m_manager_client->send_request<
        QueryAccountsService>(std::string(""));
      REQUIRE(std::ranges::any_of(results, [&] (const auto& result) {
        return result.m_account == fixture.m_manager_account;
      }));
      REQUIRE(std::ranges::any_of(results, [&] (const auto& result) {
        return result.m_account == fixture.m_trader_account;
      }));
    }
    SUBCASE("does_not_see_admin") {
      auto results = fixture.m_manager_client->send_request<
        QueryAccountsService>(std::string("admin"));
      REQUIRE(results.empty());
    }
  }

  TEST_CASE("query_accounts_trader_permissions") {
    auto fixture = Fixture();
    auto results = fixture.m_trader_client->send_request<
      QueryAccountsService>(std::string(""));
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].m_account == fixture.m_trader_account);
  }

  TEST_CASE("load_account_modification_request_summaries") {
    auto fixture = Fixture();
    auto trading_groups_root =
      fixture.m_service_locator_environment.get_root().load_directory_entry(
        DirectoryEntry::STAR_DIRECTORY, "trading_groups");
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      fixture.m_data_store.store(AccountModificationRequest(1,
        AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 12:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
      fixture.m_data_store.store(AccountModificationRequest(2,
        AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_manager_account, fixture.m_manager_account,
        time_from_string("2024-07-04 13:00:00"),
        time_from_string("2024-08-01 00:00:00")), modification);
    });
    auto query = AccountModificationRequestQuery();
    query.set_snapshot_limit(SnapshotLimit::from_head(100));
    SUBCASE("own_account") {
      query.set_index(fixture.m_trader_account);
      auto summaries = fixture.m_trader_client->send_request<
        LoadAccountModificationRequestSummariesService>(query);
      REQUIRE(summaries.size() == 1);
      REQUIRE(summaries[0].m_request.get_id() == 1);
      REQUIRE(summaries[0].m_comment_count == 0);
    }
    SUBCASE("foreign_account_is_rejected") {
      query.set_index(fixture.m_manager_account);
      REQUIRE_THROWS(fixture.m_trader_client->send_request<
        LoadAccountModificationRequestSummariesService>(query));
    }
    SUBCASE("trader_cannot_reach_root") {
      query.set_index(trading_groups_root);
      auto summaries = fixture.m_trader_client->send_request<
        LoadAccountModificationRequestSummariesService>(query);
      REQUIRE(summaries.empty());
    }
    SUBCASE("manager_loads_group") {
      query.set_index(fixture.m_trading_group.get_entry());
      auto summaries = fixture.m_manager_client->send_request<
        LoadAccountModificationRequestSummariesService>(query);
      REQUIRE(summaries.size() == 2);
    }
    SUBCASE("manager_cannot_load_unmanaged_group") {
      auto other_group = fixture.make_trading_group("other_group");
      query.set_index(other_group.get_entry());
      REQUIRE_THROWS(fixture.m_manager_client->send_request<
        LoadAccountModificationRequestSummariesService>(query));
    }
    SUBCASE("administrator_loads_root") {
      query.set_index(trading_groups_root);
      auto summaries = fixture.m_admin_client->send_request<
        LoadAccountModificationRequestSummariesService>(query);
      REQUIRE(summaries.size() == 2);
    }
  }

  TEST_CASE("summary_previous_state") {
    auto fixture = Fixture();
    auto current = RiskParameters(USD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    auto store_request = [&] (auto id, auto timestamp, const auto& parameters) {
      fixture.m_data_store.store(AccountModificationRequest(id,
        AccountModificationRequest::Type::RISK, fixture.m_trader_account,
        fixture.m_trader_account, time_from_string(timestamp),
        time_from_string("2024-08-01 00:00:00")),
        RiskModification(parameters));
    };
    auto load = [&] {
      auto query = AccountModificationRequestQuery();
      query.set_index(fixture.m_trader_account);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      return fixture.m_trader_client->send_request<
        LoadAccountModificationRequestSummariesService>(query);
    };
    SUBCASE("defaults_to_current_state") {
      auto requested = RiskParameters(USD, 200 * Money::ONE,
        RiskState::Type::ACTIVE, 20 * Money::ONE, seconds(20));
      fixture.m_data_store.with_transaction([&] {
        fixture.m_data_store.store(fixture.m_trader_account, current);
        store_request(1, "2024-07-04 12:00:00", requested);
      });
      auto summaries = load();
      REQUIRE(summaries.size() == 1);
      REQUIRE(get<RiskModification>(
        *summaries[0].m_previous_state).get_parameters() == current);
      REQUIRE(get<RiskModification>(
        *summaries[0].m_modification).get_parameters() == requested);
    }
    SUBCASE("uses_granted_predecessor") {
      auto granted = RiskParameters(USD, 200 * Money::ONE,
        RiskState::Type::ACTIVE, 20 * Money::ONE, seconds(20));
      auto requested = RiskParameters(USD, 300 * Money::ONE,
        RiskState::Type::ACTIVE, 30 * Money::ONE, seconds(30));
      fixture.m_data_store.with_transaction([&] {
        fixture.m_data_store.store(fixture.m_trader_account, current);
        store_request(1, "2024-07-04 12:00:00", granted);
        fixture.m_data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED,
          fixture.m_manager_account, 0,
          time_from_string("2024-07-04 13:00:00")));
        store_request(2, "2024-07-04 14:00:00", requested);
        fixture.m_data_store.store(2, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED,
          fixture.m_manager_account, 0,
          time_from_string("2024-07-04 15:00:00")));
      });
      auto summaries = load();
      REQUIRE(summaries.size() == 2);
      REQUIRE(get<RiskModification>(
        *summaries[1].m_previous_state).get_parameters() == granted);
    }
    SUBCASE("granted_without_a_predecessor") {
      auto requested = RiskParameters(USD, 200 * Money::ONE,
        RiskState::Type::ACTIVE, 20 * Money::ONE, seconds(20));
      fixture.m_data_store.with_transaction([&] {
        fixture.m_data_store.store(fixture.m_trader_account, current);
        store_request(1, "2024-07-04 12:00:00", requested);
        fixture.m_data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED,
          fixture.m_manager_account, 0,
          time_from_string("2024-07-04 13:00:00")));
      });
      auto summaries = load();
      REQUIRE(summaries.size() == 1);
      REQUIRE(!summaries[0].m_previous_state);
    }
    SUBCASE("pending_ignores_a_granted_predecessor") {
      auto granted = RiskParameters(USD, 200 * Money::ONE,
        RiskState::Type::ACTIVE, 20 * Money::ONE, seconds(20));
      auto requested = RiskParameters(USD, 300 * Money::ONE,
        RiskState::Type::ACTIVE, 30 * Money::ONE, seconds(30));
      fixture.m_data_store.with_transaction([&] {
        fixture.m_data_store.store(fixture.m_trader_account, current);
        store_request(1, "2024-07-04 12:00:00", granted);
        fixture.m_data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED,
          fixture.m_manager_account, 0,
          time_from_string("2024-07-04 13:00:00")));
        store_request(2, "2024-07-04 14:00:00", requested);
        fixture.m_data_store.store(2, AccountModificationRequest::Update(
          AccountModificationRequest::Status::PENDING,
          fixture.m_trader_account, 0,
          time_from_string("2024-07-04 14:00:00")));
      });
      auto summaries = load();
      REQUIRE(summaries.size() == 2);
      REQUIRE(get<RiskModification>(
        *summaries[1].m_previous_state).get_parameters() == current);
    }
  }

  TEST_CASE("summaries_mixing_entitlement_and_risk_requests") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    auto granted_parameters = RiskParameters(USD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    auto requested_parameters = RiskParameters(USD, 200 * Money::ONE,
      RiskState::Type::ACTIVE, 20 * Money::ONE, seconds(20));
    auto manager_parameters = RiskParameters(USD, 300 * Money::ONE,
      RiskState::Type::ACTIVE, 30 * Money::ONE, seconds(30));
    auto manager_requested = RiskParameters(USD, 400 * Money::ONE,
      RiskState::Type::ACTIVE, 40 * Money::ONE, seconds(40));
    fixture.m_data_store.with_transaction([&] {
      fixture.m_data_store.store(fixture.m_manager_account, manager_parameters);
      fixture.m_data_store.store(AccountModificationRequest(1,
        AccountModificationRequest::Type::RISK, fixture.m_trader_account,
        fixture.m_trader_account, time_from_string("2024-07-04 12:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        RiskModification(granted_parameters));
      fixture.m_data_store.store(1, AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED,
        fixture.m_manager_account, 0,
        time_from_string("2024-07-04 12:30:00")));
      fixture.m_data_store.store(AccountModificationRequest(2,
        AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 13:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        EntitlementModification({entitlements[0]}));
      fixture.m_data_store.store(2, AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED,
        fixture.m_manager_account, 0,
        time_from_string("2024-07-04 13:30:00")));
      fixture.m_data_store.store(AccountModificationRequest(3,
        AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_trader_account, fixture.m_trader_account,
        time_from_string("2024-07-04 14:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        EntitlementModification(entitlements));
      fixture.m_data_store.store(3, AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED, fixture.m_manager_account,
        0, time_from_string("2024-07-04 14:30:00")));
      fixture.m_data_store.store(AccountModificationRequest(4,
        AccountModificationRequest::Type::RISK, fixture.m_trader_account,
        fixture.m_trader_account, time_from_string("2024-07-04 15:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        RiskModification(requested_parameters));
      fixture.m_data_store.store(4, AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED, fixture.m_manager_account,
        0, time_from_string("2024-07-04 15:30:00")));
      fixture.m_data_store.store(AccountModificationRequest(5,
        AccountModificationRequest::Type::RISK, fixture.m_manager_account,
        fixture.m_manager_account, time_from_string("2024-07-04 16:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        RiskModification(manager_requested));
      fixture.m_data_store.store(AccountModificationRequest(6,
        AccountModificationRequest::Type::ENTITLEMENTS,
        fixture.m_manager_account, fixture.m_manager_account,
        time_from_string("2024-07-04 17:00:00"),
        time_from_string("2024-08-01 00:00:00")),
        EntitlementModification({entitlements[1]}));
    });
    auto trading_groups_root =
      fixture.m_service_locator_environment.get_root().load_directory_entry(
        DirectoryEntry::STAR_DIRECTORY, "trading_groups");
    auto query = AccountModificationRequestQuery();
    query.set_index(trading_groups_root);
    query.set_snapshot_limit(SnapshotLimit::from_head(100));
    auto summaries = fixture.m_admin_client->send_request<
      LoadAccountModificationRequestSummariesService>(query);
    REQUIRE(summaries.size() == 6);
    REQUIRE(get<EntitlementModification>(
      *summaries[2].m_modification).get_entitlements() == entitlements);
    REQUIRE(get<EntitlementModification>(
      *summaries[2].m_previous_state).get_entitlements() ==
        std::vector({entitlements[0]}));
    REQUIRE(get<RiskModification>(
      *summaries[3].m_modification).get_parameters() == requested_parameters);
    REQUIRE(get<RiskModification>(
      *summaries[3].m_previous_state).get_parameters() == granted_parameters);
    REQUIRE(get<RiskModification>(
      *summaries[4].m_modification).get_parameters() == manager_requested);
    REQUIRE(get<RiskModification>(
      *summaries[4].m_previous_state).get_parameters() == manager_parameters);
    REQUIRE(get<EntitlementModification>(
      *summaries[5].m_modification).get_entitlements() ==
        std::vector({entitlements[1]}));
    REQUIRE(get<EntitlementModification>(
      *summaries[5].m_previous_state).get_entitlements().empty());
  }

  TEST_CASE("load_account_modification_request_counts") {
    auto fixture = Fixture();
    auto modification = EntitlementModification();
    fixture.m_data_store.with_transaction([&] {
      for(auto id : {1, 2, 3}) {
        fixture.m_data_store.store(AccountModificationRequest(id,
          AccountModificationRequest::Type::ENTITLEMENTS,
          fixture.m_trader_account, fixture.m_trader_account,
          time_from_string("2024-07-04 12:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      }
      fixture.m_data_store.store(2, AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED,
        fixture.m_manager_account, 0,
        time_from_string("2024-07-04 13:00:00")));
      fixture.m_data_store.store(3, AccountModificationRequest::Update(
        AccountModificationRequest::Status::REJECTED,
        fixture.m_manager_account, 0,
        time_from_string("2024-07-04 14:00:00")));
    });
    auto query = AccountModificationRequestQuery();
    query.set_index(fixture.m_trader_account);
    query.set_snapshot_limit(SnapshotLimit::from_head(1));
    auto counts = fixture.m_trader_client->send_request<
      LoadAccountModificationRequestCountsService>(query);
    REQUIRE(counts == AccountModificationRequestCounts(1, 1, 1));
  }
}
