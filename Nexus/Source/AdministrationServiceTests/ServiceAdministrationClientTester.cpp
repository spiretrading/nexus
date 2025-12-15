#include <memory>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestServiceAdministrationClient =
      ServiceAdministrationClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestServiceAdministrationClient> m_client;

    Fixture() {
      register_administration_services(out(m_server.get_slots()));
      register_administration_messages(out(m_server.get_slots()));
      m_client = make_client<TestServiceAdministrationClient>();
    }

    void close_server_side() {
      auto close_token = Async<void>();
      on_request<CheckAdministratorService>(
        [&] (auto& request, const DirectoryEntry&) {
          request.set(false);
          request.get_client().close();
          close_token.get_eval().set();
        });
      try {
        m_client->check_administrator(DirectoryEntry());
      } catch(const std::exception&) {}
      close_token.get();
    }
  };
}

TEST_SUITE("ServiceAdministrationClient") {
  TEST_CASE("load_accounts_by_roles") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(16, "entitled_account");
    auto roles = AccountRoles();
    auto accounts =
      std::vector{DirectoryEntry::make_account(1, "test_account")};
    fixture.on_request<LoadAccountsByRolesService>(
      [&] (auto& request, const auto& received_roles) {
        REQUIRE(received_roles == roles);
        request.set(accounts);
      });
    auto received_accounts =
      REQUIRE_NO_THROW(fixture.m_client->load_accounts_by_roles(roles));
    REQUIRE(received_accounts == accounts);
  }

  TEST_CASE("load_administrators_root_entry") {
    auto fixture = Fixture();
    auto entry = DirectoryEntry::make_directory(1, "admin_root");
    fixture.on_request<LoadAdministratorsRootEntryService>([&] (auto& request) {
      request.set(entry);
    });
    auto received_entry =
      REQUIRE_NO_THROW(fixture.m_client->load_administrators_root_entry());
    REQUIRE(received_entry == entry);
  }

  TEST_CASE("load_services_root_entry") {
    auto fixture = Fixture();
    auto entry = DirectoryEntry::make_directory(2, "services_root");
    fixture.on_request<LoadServicesRootEntryService>([&] (auto& request) {
      request.set(entry);
    });
    auto received_entry =
      REQUIRE_NO_THROW(fixture.m_client->load_services_root_entry());
    REQUIRE(received_entry == entry);
  }

  TEST_CASE("load_trading_groups_root_entry") {
    auto fixture = Fixture();
    auto entry = DirectoryEntry::make_directory(3, "trading_groups_root");
    fixture.on_request<LoadTradingGroupsRootEntryService>([&] (auto& request) {
      request.set(entry);
    });
    auto received_entry =
      REQUIRE_NO_THROW(fixture.m_client->load_trading_groups_root_entry());
    REQUIRE(received_entry == entry);
  }

  TEST_CASE("check_administrator") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(4, "admin_account");
    fixture.on_request<CheckAdministratorService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.set(true);
      });
    auto is_administrator =
      REQUIRE_NO_THROW(fixture.m_client->check_administrator(account));
    REQUIRE(is_administrator);
  }

  TEST_CASE("load_account_roles") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(5, "account");
    auto roles = AccountRoles();
    roles.set(AccountRole::MANAGER);
    fixture.on_request<LoadAccountRolesService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.set(roles);
      });
    auto received_roles =
      REQUIRE_NO_THROW(fixture.m_client->load_account_roles(account));
    REQUIRE(received_roles == roles);
  }

  TEST_CASE("load_supervised_account_roles") {
    auto fixture = Fixture();
    auto parent = DirectoryEntry::make_account(6, "parent_account");
    auto child = DirectoryEntry::make_account(7, "child_account");
    auto roles = AccountRoles();
    roles.set(AccountRole::TRADER);
    fixture.on_request<LoadSupervisedAccountRolesService>(
      [&] (auto& request, const auto& received_parent,
          const auto& received_child) {
        REQUIRE(received_parent == parent);
        REQUIRE(received_child == child);
        request.set(roles);
      });
    auto received_roles =
      REQUIRE_NO_THROW(fixture.m_client->load_account_roles(parent, child));
    REQUIRE(received_roles == roles);
  }

  TEST_CASE("load_parent_trading_group") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(8, "trader_account");
    auto parent_group = DirectoryEntry::make_account(9, "parent_group");
    fixture.on_request<LoadParentTradingGroupService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.set(parent_group);
      });
    auto received_parent_group =
      REQUIRE_NO_THROW(fixture.m_client->load_parent_trading_group(account));
    REQUIRE(received_parent_group == parent_group);
  }

  TEST_CASE("load_identity") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(10, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "John";
    identity.m_last_name = "Doe";
    fixture.on_request<LoadAccountIdentityService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.set(identity);
      });
    auto received_identity =
      REQUIRE_NO_THROW(fixture.m_client->load_identity(account));
    REQUIRE(received_identity.m_first_name == identity.m_first_name);
    REQUIRE(received_identity.m_last_name == identity.m_last_name);
  }

  TEST_CASE("store_identity") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(11, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "Jane";
    identity.m_last_name = "Smith";
    fixture.on_request<StoreAccountIdentityService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_identity) {
        REQUIRE(received_account == account);
        REQUIRE(received_identity.m_first_name == identity.m_first_name);
        REQUIRE(received_identity.m_last_name == identity.m_last_name);
        request.set();
      });
    REQUIRE_NOTHROW(fixture.m_client->store(account, identity));
  }

  TEST_CASE("load_trading_group") {
    auto fixture = Fixture();
    auto directory = DirectoryEntry::make_directory(1, "trading_group");
    auto trading_group = TradingGroup(directory,
      DirectoryEntry::make_directory(2, "managers"),
      {DirectoryEntry::make_account(3, "manager1")},
      DirectoryEntry::make_directory(4, "traders"),
      {DirectoryEntry::make_account(5, "trader1")});
    fixture.on_request<LoadTradingGroupService>(
      [&] (auto& request, const auto& received_directory) {
        REQUIRE(received_directory == directory);
        request.set(trading_group);
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
    auto account = DirectoryEntry::make_account(6, "manager_account");
    auto managed_groups = std::vector{DirectoryEntry::make_account(7, "group1"),
      DirectoryEntry::make_account(8, "group2")};
    fixture.on_request<LoadManagedTradingGroupsService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.set(managed_groups);
      });
    auto received_groups =
      REQUIRE_NO_THROW(fixture.m_client->load_managed_trading_groups(account));
    REQUIRE(received_groups == managed_groups);
  }

  TEST_CASE("load_administrators") {
    auto fixture = Fixture();
    auto administrators = std::vector{DirectoryEntry::make_account(9, "admin1"),
      DirectoryEntry::make_account(10, "admin2")};
    fixture.on_request<LoadAdministratorsService>([&] (auto& request) {
      request.set(administrators);
    });
    auto received_administrators =
      REQUIRE_NO_THROW(fixture.m_client->load_administrators());
    REQUIRE(received_administrators == administrators);
  }

  TEST_CASE("load_services") {
    auto fixture = Fixture();
    auto services = std::vector{DirectoryEntry::make_account(11, "service1"),
      DirectoryEntry::make_account(12, "service2")};
    fixture.on_request<LoadServicesService>([&] (auto& request) {
      request.set(services);
    });
    auto received_services =
      REQUIRE_NO_THROW(fixture.m_client->load_services());
    REQUIRE(received_services == services);
  }

  TEST_CASE("load_entitlements") {
    auto fixture = Fixture();
    auto entitlements = EntitlementDatabase();
    entitlements.add({"Entitlement1", Money(100), AUD,
      DirectoryEntry::make_account(1, "group1"), {{EntitlementKey(ASX),
        MarketDataTypeSet({MarketDataType::BBO_QUOTE})}}});
    entitlements.add({"Entitlement2", Money(200), CAD,
      DirectoryEntry::make_account(2, "group2"), {{EntitlementKey(TSX),
        MarketDataTypeSet({MarketDataType::BOOK_QUOTE})}}});
    fixture.on_request<LoadEntitlementsService>([&] (auto& request) {
      request.set(entitlements);
    });
    auto received_entitlements =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlements());
    test_json_equality(received_entitlements, entitlements);
  }

  TEST_CASE("load_account_entitlements") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(13, "entitled_account");
    auto entitlements = std::vector{
      DirectoryEntry::make_account(14, "entitlement1"),
      DirectoryEntry::make_account(15, "entitlement2")};
    fixture.on_request<LoadAccountEntitlementsService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account);
        request.set(entitlements);
      });
    auto received_entitlements =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlements(account));
    REQUIRE(received_entitlements == entitlements);
  }

  TEST_CASE("store_entitlements") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(16, "entitled_account");
    auto entitlements = std::vector{
      DirectoryEntry::make_account(17, "entitlement1"),
      DirectoryEntry::make_account(18, "entitlement2")};
    fixture.on_request<StoreEntitlementsService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_entitlements) {
        REQUIRE(received_account == account);
        REQUIRE(received_entitlements == entitlements);
        request.set();
      });
    REQUIRE_NOTHROW(
      fixture.m_client->store_entitlements(account, entitlements));
  }

  TEST_CASE("get_risk_parameters_publisher") {
    auto fixture = Fixture();
    auto account_a = DirectoryEntry::make_account(55, "Alexis");
    auto parameters_a = std::make_shared<Queue<RiskParameters>>();
    auto account_a_parameters = RiskParameters(AUD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    fixture.on_request<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_a);
        request.set(account_a_parameters);
      });
    REQUIRE_NO_THROW(fixture.m_client->get_risk_parameters_publisher(
      account_a).monitor(parameters_a));
    REQUIRE(account_a_parameters == parameters_a->pop());
    auto account_b = DirectoryEntry::make_account(66, "Aurora");
    auto parameters_b = std::make_shared<Queue<RiskParameters>>();
    auto account_b_parameters = RiskParameters(CAD, 200 * Money::ONE,
      RiskState::Type::ACTIVE, Money::ONE, seconds(90));
    auto server_side_client =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_b);
        server_side_client = &request.get_client();
        request.set(account_b_parameters);
      });
    REQUIRE_NO_THROW(fixture.m_client->get_risk_parameters_publisher(
      account_b).monitor(parameters_b));
    REQUIRE(account_b_parameters == parameters_b->pop());
    account_a_parameters.m_buying_power = 123 * Money::ONE;
    send_record_message<RiskParametersMessage>(
      *server_side_client, account_a, account_a_parameters);
    REQUIRE(account_a_parameters == parameters_a->pop());
    account_b_parameters.m_buying_power = 567 * Money::ONE;
    send_record_message<RiskParametersMessage>(
      *server_side_client, account_b, account_b_parameters);
    REQUIRE(account_b_parameters == parameters_b->pop());
    auto updated_server_side_client =
      Async<TestServiceProtocolServer::ServiceProtocolClient*>();
    auto recovered_accounts = std::unordered_set<DirectoryEntry>();
    fixture.on_request<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(!recovered_accounts.contains(received_account));
        recovered_accounts.insert(received_account);
        if(received_account == account_a) {
          request.set(account_a_parameters);
        } else if(received_account == account_b) {
          request.set(account_b_parameters);
        } else {
          REQUIRE(false);
        }
        if(recovered_accounts.size() == 2) {
          updated_server_side_client.get_eval().set(&request.get_client());
        }
      });
    fixture.close_server_side();
    server_side_client = updated_server_side_client.get();
    REQUIRE(recovered_accounts.contains(account_a));
    REQUIRE(recovered_accounts.contains(account_b));
    account_a_parameters.m_buying_power = 456 * Money::ONE;
    send_record_message<RiskParametersMessage>(
      *server_side_client, account_a, account_a_parameters);
    REQUIRE(account_a_parameters == parameters_a->pop());
    account_b_parameters.m_buying_power = 789 * Money::ONE;
    send_record_message<RiskParametersMessage>(
      *server_side_client, account_b, account_b_parameters);
    REQUIRE(account_b_parameters == parameters_b->pop());
  }

  TEST_CASE("store_risk_parameters") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(19, "risk_account");
    auto parameters = RiskParameters(AUD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    fixture.on_request<StoreRiskParametersService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_parameters) {
        REQUIRE(received_account == account);
        REQUIRE(received_parameters == parameters);
        request.set();
      });
    REQUIRE_NOTHROW(
      fixture.m_client->store(account, parameters));
  }

  TEST_CASE("get_risk_state_publisher") {
    auto fixture = Fixture();
    auto account_a = DirectoryEntry::make_account(58, "Alexis");
    auto states_a = std::make_shared<Queue<RiskState>>();
    auto account_a_state = RiskState(RiskState::Type::ACTIVE);
    fixture.on_request<MonitorRiskStateService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_a);
        request.set(account_a_state);
      });
    REQUIRE_NO_THROW(
      fixture.m_client->get_risk_state_publisher(account_a).monitor(states_a));
    REQUIRE(account_a_state == states_a->pop());
    auto account_b = DirectoryEntry::make_account(59, "Aurora");
    auto states_b = std::make_shared<Queue<RiskState>>();
    auto account_b_state = RiskState(RiskState::Type::DISABLED);
    auto server_side_client =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<MonitorRiskStateService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_b);
        server_side_client = &request.get_client();
        request.set(account_b_state);
      });
    REQUIRE_NO_THROW(
      fixture.m_client->get_risk_state_publisher(account_b).monitor(states_b));
    REQUIRE(account_b_state == states_b->pop());
    account_a_state.m_type = RiskState::Type::DISABLED;
    send_record_message<RiskStateMessage>(
      *server_side_client, account_a, account_a_state);
    REQUIRE(account_a_state == states_a->pop());
    account_b_state.m_type = RiskState::Type::ACTIVE;
    send_record_message<RiskStateMessage>(
      *server_side_client, account_b, account_b_state);
    REQUIRE(account_b_state == states_b->pop());
    auto updated_server_side_client =
      Async<TestServiceProtocolServer::ServiceProtocolClient*>();
    auto recovered_accounts = std::unordered_set<DirectoryEntry>();
    fixture.on_request<MonitorRiskStateService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(!recovered_accounts.contains(received_account));
        recovered_accounts.insert(received_account);
        if(received_account == account_a) {
          request.set(account_a_state);
        } else if(received_account == account_b) {
          request.set(account_b_state);
        } else {
          REQUIRE(false);
        }
        if(recovered_accounts.size() == 2) {
          updated_server_side_client.get_eval().set(&request.get_client());
        }
      });
    fixture.close_server_side();
    server_side_client = updated_server_side_client.get();
    REQUIRE(recovered_accounts.contains(account_a));
    REQUIRE(recovered_accounts.contains(account_b));
    account_a_state.m_type = RiskState::Type::ACTIVE;
    send_record_message<RiskStateMessage>(
      *server_side_client, account_a, account_a_state);
    REQUIRE(account_a_state == states_a->pop());
    account_b_state.m_type = RiskState::Type::DISABLED;
    send_record_message<RiskStateMessage>(
      *server_side_client, account_b, account_b_state);
    REQUIRE(account_b_state == states_b->pop());
  }

  TEST_CASE("store_risk_state") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(19, "risk_account");
    auto risk_state = RiskState(RiskState::Type::ACTIVE);
    fixture.on_request<StoreRiskStateService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_risk_state) {
        REQUIRE(received_account == account);
        REQUIRE(received_risk_state == risk_state);
        request.set();
      });
    REQUIRE_NOTHROW(fixture.m_client->store(account, risk_state));
  }

  TEST_CASE("load_account_modification_request") {
    auto fixture = Fixture();
    auto id = 20;
    auto account = DirectoryEntry::make_account(21, "mod_account");
    auto submission_account = DirectoryEntry::make_account(22, "sub_account");
    auto timestamp = ptime(gregorian::date(2024, 5, 20));
    auto request_data = AccountModificationRequest(id,
      AccountModificationRequest::Type::ENTITLEMENTS, account,
      submission_account, timestamp);
    fixture.on_request<LoadAccountModificationRequestService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.set(request_data);
      });
    auto received_request =
      REQUIRE_NO_THROW(fixture.m_client->load_account_modification_request(id));
    test_json_equality(received_request, request_data);
  }

  TEST_CASE("load_account_modification_request_ids") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(23, "mod_account");
    auto start_id = 24;
    auto max_count = 10;
    auto ids = std::vector<AccountModificationRequest::Id>{25, 26, 27};
    fixture.on_request<LoadAccountModificationRequestIdsService>(
      [&] (auto& request, const auto& received_account, auto received_start_id,
          auto received_max_count) {
        REQUIRE(received_account == account);
        REQUIRE(received_start_id == start_id);
        REQUIRE(received_max_count == max_count);
        request.set(ids);
      });
    auto received_ids =
      REQUIRE_NO_THROW(fixture.m_client->load_account_modification_request_ids(
        account, start_id, max_count));
    REQUIRE(received_ids == ids);
  }

  TEST_CASE("load_managed_account_modification_request_ids") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(28, "manager_account");
    auto start_id = 29;
    auto max_count = 5;
    auto ids = std::vector<AccountModificationRequest::Id>{30, 31};
    fixture.on_request<LoadManagedAccountModificationRequestIdsService>(
      [&] (auto& request, const auto& received_account, auto received_start_id,
          auto received_max_count) {
        REQUIRE(received_account == account);
        REQUIRE(received_start_id == start_id);
        REQUIRE(received_max_count == max_count);
        request.set(ids);
      });
    auto received_ids = REQUIRE_NO_THROW(
      fixture.m_client->load_managed_account_modification_request_ids(
        account, start_id, max_count));
    REQUIRE(received_ids == ids);
  }

  TEST_CASE("load_entitlement_modification") {
    auto fixture = Fixture();
    auto id = 32;
    auto entitlements = std::vector{
      DirectoryEntry::make_account(33, "entitlementA"),
      DirectoryEntry::make_account(34, "entitlementB")};
    auto modification = EntitlementModification(entitlements);
    fixture.on_request<LoadEntitlementModificationService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.set(modification);
      });
    auto received_modification =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlement_modification(id));
    test_json_equality(received_modification, modification);
  }

  TEST_CASE("submit_modification_request") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(35, "mod_account");
    auto entitlements =
      std::vector{DirectoryEntry::make_account(36, "entitlement")};
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message();
    auto request_data = AccountModificationRequest(
      37, AccountModificationRequest::Type::ENTITLEMENTS, account,
      DirectoryEntry::make_account(38, "sub_account"),
      ptime(gregorian::date(2024, 5, 21)));
    fixture.on_request<SubmitEntitlementModificationRequestService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_modification, const auto& received_comment) {
        REQUIRE(received_account == account);
        test_json_equality(received_modification, modification);
        request.set(request_data);
      });
    auto received_request = REQUIRE_NO_THROW(
      fixture.m_client->submit(account, modification, comment));
    test_json_equality(received_request, request_data);
  }

  TEST_CASE("load_risk_modification") {
    auto fixture = Fixture();
    auto id = 39;
    auto risk_parameters = RiskParameters(AUD, 1000 * Money::ONE,
      RiskState::Type::ACTIVE, 100 * Money::ONE, seconds(30));
    auto modification = RiskModification(risk_parameters);
    fixture.on_request<LoadRiskModificationService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.set(modification);
      });
    auto received_modification =
      REQUIRE_NO_THROW(fixture.m_client->load_risk_modification(id));
    test_json_equality(received_modification, modification);
  }

  TEST_CASE("submit_risk_modification_request") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(40, "risk_mod_account");
    auto risk_parameters = RiskParameters(CAD, 500 * Money::ONE,
      RiskState::Type::ACTIVE, 50 * Money::ONE, seconds(60));
    auto modification = RiskModification(risk_parameters);
    auto comment = Nexus::Message();
    auto request_data = AccountModificationRequest(
      41, AccountModificationRequest::Type::RISK, account,
      DirectoryEntry::make_account(42, "risk_sub_account"),
      ptime(gregorian::date(2024, 5, 22)));
    fixture.on_request<SubmitRiskModificationRequestService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_modification, const auto& received_comment) {
        REQUIRE(received_account == account);
        test_json_equality(received_modification, modification);
        request.set(request_data);
      });
    auto received_request = REQUIRE_NO_THROW(
      fixture.m_client->submit(account, modification, comment));
    test_json_equality(received_request, request_data);
  }

  TEST_CASE("load_account_modification_request_status") {
    auto fixture = Fixture();
    auto id = 43;
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::PENDING,
      DirectoryEntry::make_account(44, "status_account"), 1,
      ptime(gregorian::date(2024, 5, 23)));
    fixture.on_request<LoadAccountModificationRequestStatusService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.set(update);
      });
    auto received_update = REQUIRE_NO_THROW(
      fixture.m_client->load_account_modification_request_status(id));
    REQUIRE(received_update == update);
  }

  TEST_CASE("approve_account_modification_request") {
    auto fixture = Fixture();
    auto id = 45;
    auto comment = Nexus::Message();
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::make_account(46, "approver_account"), 2,
      ptime(gregorian::date(2024, 5, 24)));
    fixture.on_request<ApproveAccountModificationRequestService>(
      [&] (auto& request, auto received_id, const auto& received_comment) {
        REQUIRE(received_id == id);
        request.set(update);
      });
    auto received_update = REQUIRE_NO_THROW(
      fixture.m_client->approve_account_modification_request(id, comment));
    REQUIRE(received_update == update);
  }

  TEST_CASE("reject_account_modification_request") {
    auto fixture = Fixture();
    auto id = 47;
    auto comment = Nexus::Message();
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::REJECTED,
      DirectoryEntry::make_account(48, "rejecter_account"), 3,
      ptime(gregorian::date(2024, 5, 25)));
    fixture.on_request<RejectAccountModificationRequestService>(
      [&] (auto& request, auto received_id, const auto& received_comment) {
        REQUIRE(received_id == id);
        request.set(update);
      });
    auto received_update = REQUIRE_NO_THROW(
      fixture.m_client->reject_account_modification_request(id, comment));
    REQUIRE(received_update == update);
  }

  TEST_CASE("load_message") {
    auto fixture = Fixture();
    auto id = 49;
    auto message =
      Nexus::Message(id, DirectoryEntry::make_account(50, "msg_account"),
        ptime(gregorian::date(2024, 5, 26)), {});
    fixture.on_request<LoadMessageService>([&] (auto& request, auto received_id) {
      REQUIRE(received_id == id);
      request.set(message);
    });
    auto received_message =
      REQUIRE_NO_THROW(fixture.m_client->load_message(id));
    test_json_equality(received_message, message);
  }

  TEST_CASE("load_message_ids") {
    auto fixture = Fixture();
    auto id = 51;
    auto ids = std::vector<Nexus::Message::Id>{52, 53, 54};
    fixture.on_request<LoadMessageIdsService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.set(ids);
      });
    auto received_ids =
      REQUIRE_NO_THROW(fixture.m_client->load_message_ids(id));
    REQUIRE(received_ids == ids);
  }

  TEST_CASE("send_account_modification_request_message") {
    auto fixture = Fixture();
    auto id = 55;
    auto message = Nexus::Message();
    auto appended_message = Nexus::Message(
      56, DirectoryEntry::make_account(57, "sender_account"),
      ptime(gregorian::date(2024, 5, 27)), {});
    fixture.on_request<SendAccountModificationRequestMessageService>(
      [&] (auto& request, auto received_id, const auto& received_message) {
        REQUIRE(received_id == id);
        request.set(appended_message);
      });
    auto received_message = REQUIRE_NO_THROW(
      fixture.m_client->send_account_modification_request_message(id, message));
    test_json_equality(received_message, appended_message);
  }
}
