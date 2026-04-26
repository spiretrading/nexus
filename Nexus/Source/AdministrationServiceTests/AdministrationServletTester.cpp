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

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaAdministrationServlet<
        ServiceLocatorClient, LocalAdministrationDataStore*, FixedTimeClient*,
        TriggerTimer*>>;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    LocalAdministrationDataStore m_data_store;
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
      auto trading_group = service_locator.make_directory(
        "test_trading_group", trading_groups_root);
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
}
