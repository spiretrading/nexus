#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
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
        ServiceLocatorClient, LocalAdministrationDataStore*, FixedTimeClient*>>;
    FixedTimeClient m_time_client;
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
          &m_data_store, &m_time_client)), m_server_connection,
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

  TEST_CASE("store_and_load_account_entitlements") {
    auto fixture = Fixture();
    auto entitlements = std::vector<DirectoryEntry>();
    for(auto& entry : fixture.m_entitlements.get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    SUBCASE("admin") {
      REQUIRE_NOTHROW(
        fixture.m_admin_client->send_request<StoreEntitlementsService>(
          fixture.m_trader_account, entitlements));
      auto result = fixture.m_admin_client->send_request<
        LoadAccountEntitlementsService>(fixture.m_trader_account);
      REQUIRE(result.size() == entitlements.size());
      for(auto& entitlement : entitlements) {
        REQUIRE(
          std::find(result.begin(), result.end(), entitlement) != result.end());
      }
      REQUIRE_NOTHROW(
        fixture.m_admin_client->send_request<StoreEntitlementsService>(
          fixture.m_trader_account, std::vector<DirectoryEntry>()));
      result = fixture.m_admin_client->send_request<
        LoadAccountEntitlementsService>(fixture.m_trader_account);
      REQUIRE(result.empty());
    }
    SUBCASE("trader") {
      REQUIRE_THROWS_AS(
        fixture.m_trader_client->send_request<StoreEntitlementsService>(
          fixture.m_trader_account, entitlements), ServiceRequestException);
    }
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
          fixture.m_trader_account, modification, comment);
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
          DirectoryEntry(), modification, comment);
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
          request.get_id(), review_comment);
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
          ApproveAccountModificationRequestService>(request.get_id(), comment);
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
          fixture.m_trader_account, modification, comment);
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
          DirectoryEntry(), modification, comment);
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
}
