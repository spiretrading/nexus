#include <memory>
#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  struct Fixture {
    using TestServiceAdministrationClient =
      ServiceAdministrationClient<TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceAdministrationClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterAdministrationServices(Store(m_server.GetSlots()));
      RegisterAdministrationMessages(Store(m_server.GetSlots()));
      auto builder = TestServiceProtocolClientBuilder([=, this] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client = std::make_unique<TestServiceAdministrationClient>(builder);
    }

    template<typename T, typename F>
    void handle(F&& handler) {
      using Slot = typename Services::Details::GetSlotType<RequestToken<
        TestServiceProtocolServer::ServiceProtocolClient, T>>::type;
      auto& stored_handler = m_handlers[typeid(T)];
      if(stored_handler) {
        *std::static_pointer_cast<Slot>(stored_handler) =
          std::forward<F>(handler);
      } else {
        auto shared_handler = std::make_shared<Slot>(std::forward<F>(handler));
        stored_handler = shared_handler;
        T::AddRequestSlot(Store(m_server.GetSlots()),
          [handler = std::move(shared_handler)] (auto&&... args) {
            try {
              (*handler)(std::forward<decltype(args)>(args)...);
            } catch(...) {
              throw ServiceRequestException("Test failed.");
            }
          });
      }
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
    REQUIRE_NOTHROW(fixture.m_client->store(account, identity));
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
    entitlements.add({"Entitlement1", Money(100), AUD,
      DirectoryEntry::MakeAccount(1, "group1"), {{EntitlementKey(ASX),
        MarketDataTypeSet({MarketDataType::BBO_QUOTE})}}});
    entitlements.add({"Entitlement2", Money(200), CAD,
      DirectoryEntry::MakeAccount(2, "group2"), {{EntitlementKey(TSX),
        MarketDataTypeSet({MarketDataType::BOOK_QUOTE})}}});
    fixture.handle<LoadEntitlementsService>([&] (auto& request) {
      request.SetResult(entitlements);
    });
    auto received_entitlements =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlements());
    TestJsonEquality(received_entitlements, entitlements);
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

  TEST_CASE("get_risk_parameters_publisher") {
    auto fixture = Fixture();
    auto account_a = DirectoryEntry::MakeAccount(55, "Alexis");
    auto parameters_a = std::make_shared<Queue<RiskParameters>>();
    auto account_a_parameters = RiskParameters(AUD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    fixture.handle<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_a);
        request.SetResult(account_a_parameters);
      });
    REQUIRE_NO_THROW(fixture.m_client->get_risk_parameters_publisher(
      account_a).Monitor(parameters_a));
    REQUIRE(account_a_parameters == parameters_a->Pop());
    auto account_b = DirectoryEntry::MakeAccount(66, "Aurora");
    auto parameters_b = std::make_shared<Queue<RiskParameters>>();
    auto account_b_parameters = RiskParameters(CAD, 200 * Money::ONE,
      RiskState::Type::ACTIVE, Money::ONE, seconds(90));
    auto server_side_client =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.handle<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_b);
        request.SetResult(account_b_parameters);
        server_side_client = &request.GetClient();
      });
    REQUIRE_NO_THROW(fixture.m_client->get_risk_parameters_publisher(
      account_b).Monitor(parameters_b));
    REQUIRE(account_b_parameters == parameters_b->Pop());
    account_a_parameters.m_buying_power = 123 * Money::ONE;
    SendRecordMessage<RiskParametersMessage>(
      *server_side_client, account_a, account_a_parameters);
    REQUIRE(account_a_parameters == parameters_a->Pop());
    account_b_parameters.m_buying_power = 567 * Money::ONE;
    SendRecordMessage<RiskParametersMessage>(
      *server_side_client, account_b, account_b_parameters);
    REQUIRE(account_b_parameters == parameters_b->Pop());
    auto updated_server_side_client =
      Async<TestServiceProtocolServer::ServiceProtocolClient*>();
    auto recovered_accounts = std::unordered_set<DirectoryEntry>();
    fixture.handle<MonitorRiskParametersService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(!recovered_accounts.contains(received_account));
        recovered_accounts.insert(received_account);
        if(received_account == account_a) {
          request.SetResult(account_a_parameters);
        } else if(received_account == account_b) {
          request.SetResult(account_b_parameters);
        } else {
          REQUIRE(false);
        }
        if(recovered_accounts.size() == 2) {
          updated_server_side_client.GetEval().SetResult(&request.GetClient());
        }
      });
    server_side_client->Close();
    server_side_client = updated_server_side_client.Get();
    REQUIRE(recovered_accounts.contains(account_a));
    REQUIRE(recovered_accounts.contains(account_b));
    account_a_parameters.m_buying_power = 456 * Money::ONE;
    SendRecordMessage<RiskParametersMessage>(
      *server_side_client, account_a, account_a_parameters);
    REQUIRE(account_a_parameters == parameters_a->Pop());
    account_b_parameters.m_buying_power = 789 * Money::ONE;
    SendRecordMessage<RiskParametersMessage>(
      *server_side_client, account_b, account_b_parameters);
    REQUIRE(account_b_parameters == parameters_b->Pop());
  }

  TEST_CASE("store_risk_parameters") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(19, "risk_account");
    auto parameters = RiskParameters(AUD, 100 * Money::ONE,
      RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
    fixture.handle<StoreRiskParametersService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_parameters) {
        REQUIRE(received_account == account);
        REQUIRE(received_parameters == parameters);
        request.SetResult();
      });
    REQUIRE_NOTHROW(
      fixture.m_client->store(account, parameters));
  }

  TEST_CASE("get_risk_state_publisher") {
    auto fixture = Fixture();
    auto account_a = DirectoryEntry::MakeAccount(58, "Alexis");
    auto states_a = std::make_shared<Queue<RiskState>>();
    auto account_a_state = RiskState(RiskState::Type::ACTIVE);
    fixture.handle<MonitorRiskStateService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_a);
        request.SetResult(account_a_state);
      });
    REQUIRE_NO_THROW(
      fixture.m_client->get_risk_state_publisher(account_a).Monitor(states_a));
    REQUIRE(account_a_state == states_a->Pop());
    auto account_b = DirectoryEntry::MakeAccount(59, "Aurora");
    auto states_b = std::make_shared<Queue<RiskState>>();
    auto account_b_state = RiskState(RiskState::Type::DISABLED);
    auto server_side_client =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.handle<MonitorRiskStateService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(received_account == account_b);
        request.SetResult(account_b_state);
        server_side_client = &request.GetClient();
      });
    REQUIRE_NO_THROW(
      fixture.m_client->get_risk_state_publisher(account_b).Monitor(states_b));
    REQUIRE(account_b_state == states_b->Pop());
    account_a_state.m_type = RiskState::Type::DISABLED;
    SendRecordMessage<RiskStateMessage>(
      *server_side_client, account_a, account_a_state);
    REQUIRE(account_a_state == states_a->Pop());
    account_b_state.m_type = RiskState::Type::ACTIVE;
    SendRecordMessage<RiskStateMessage>(
      *server_side_client, account_b, account_b_state);
    REQUIRE(account_b_state == states_b->Pop());
    auto updated_server_side_client =
      Async<TestServiceProtocolServer::ServiceProtocolClient*>();
    auto recovered_accounts = std::unordered_set<DirectoryEntry>();
    fixture.handle<MonitorRiskStateService>(
      [&] (auto& request, const auto& received_account) {
        REQUIRE(!recovered_accounts.contains(received_account));
        recovered_accounts.insert(received_account);
        if(received_account == account_a) {
          request.SetResult(account_a_state);
        } else if(received_account == account_b) {
          request.SetResult(account_b_state);
        } else {
          REQUIRE(false);
        }
        if(recovered_accounts.size() == 2) {
          updated_server_side_client.GetEval().SetResult(&request.GetClient());
        }
      });
    server_side_client->Close();
    server_side_client = updated_server_side_client.Get();
    REQUIRE(recovered_accounts.contains(account_a));
    REQUIRE(recovered_accounts.contains(account_b));
    account_a_state.m_type = RiskState::Type::ACTIVE;
    SendRecordMessage<RiskStateMessage>(
      *server_side_client, account_a, account_a_state);
    REQUIRE(account_a_state == states_a->Pop());
    account_b_state.m_type = RiskState::Type::DISABLED;
    SendRecordMessage<RiskStateMessage>(
      *server_side_client, account_b, account_b_state);
    REQUIRE(account_b_state == states_b->Pop());
  }

  TEST_CASE("store_risk_state") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(19, "risk_account");
    auto risk_state = RiskState(RiskState::Type::ACTIVE);
    fixture.handle<StoreRiskStateService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_risk_state) {
        REQUIRE(received_account == account);
        REQUIRE(received_risk_state == risk_state);
        request.SetResult();
      });
    REQUIRE_NOTHROW(fixture.m_client->store(account, risk_state));
  }

  TEST_CASE("load_account_modification_request") {
    auto fixture = Fixture();
    auto id = 20;
    auto account = DirectoryEntry::MakeAccount(21, "mod_account");
    auto submission_account = DirectoryEntry::MakeAccount(22, "sub_account");
    auto timestamp = ptime(gregorian::date(2024, 5, 20));
    auto request_data = AccountModificationRequest(id,
      AccountModificationRequest::Type::ENTITLEMENTS, account,
      submission_account, timestamp);
    fixture.handle<LoadAccountModificationRequestService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.SetResult(request_data);
      });
    auto received_request =
      REQUIRE_NO_THROW(fixture.m_client->load_account_modification_request(id));
    TestJsonEquality(received_request, request_data);
  }

  TEST_CASE("load_account_modification_request_ids") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(23, "mod_account");
    auto start_id = 24;
    auto max_count = 10;
    auto ids = std::vector<AccountModificationRequest::Id>{25, 26, 27};
    fixture.handle<LoadAccountModificationRequestIdsService>(
      [&] (auto& request, const auto& received_account, auto received_start_id,
          auto received_max_count) {
        REQUIRE(received_account == account);
        REQUIRE(received_start_id == start_id);
        REQUIRE(received_max_count == max_count);
        request.SetResult(ids);
      });
    auto received_ids =
      REQUIRE_NO_THROW(fixture.m_client->load_account_modification_request_ids(
        account, start_id, max_count));
    REQUIRE(received_ids == ids);
  }

  TEST_CASE("load_managed_account_modification_request_ids") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(28, "manager_account");
    auto start_id = 29;
    auto max_count = 5;
    auto ids = std::vector<AccountModificationRequest::Id>{30, 31};
    fixture.handle<LoadManagedAccountModificationRequestIdsService>(
      [&] (auto& request, const auto& received_account, auto received_start_id,
          auto received_max_count) {
        REQUIRE(received_account == account);
        REQUIRE(received_start_id == start_id);
        REQUIRE(received_max_count == max_count);
        request.SetResult(ids);
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
      DirectoryEntry::MakeAccount(33, "entitlementA"),
      DirectoryEntry::MakeAccount(34, "entitlementB")};
    auto modification = EntitlementModification(entitlements);
    fixture.handle<LoadEntitlementModificationService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.SetResult(modification);
      });
    auto received_modification =
      REQUIRE_NO_THROW(fixture.m_client->load_entitlement_modification(id));
    TestJsonEquality(received_modification, modification);
  }

  TEST_CASE("submit_modification_request") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(35, "mod_account");
    auto entitlements =
      std::vector{DirectoryEntry::MakeAccount(36, "entitlement")};
    auto modification = EntitlementModification(entitlements);
    auto comment = Nexus::Message();
    auto request_data = AccountModificationRequest(
      37, AccountModificationRequest::Type::ENTITLEMENTS, account,
      DirectoryEntry::MakeAccount(38, "sub_account"),
      ptime(gregorian::date(2024, 5, 21)));
    fixture.handle<SubmitEntitlementModificationRequestService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_modification, const auto& received_comment) {
        REQUIRE(received_account == account);
        TestJsonEquality(received_modification, modification);
        request.SetResult(request_data);
      });
    auto received_request = REQUIRE_NO_THROW(
      fixture.m_client->submit(account, modification, comment));
    TestJsonEquality(received_request, request_data);
  }

  TEST_CASE("load_risk_modification") {
    auto fixture = Fixture();
    auto id = 39;
    auto risk_parameters = RiskParameters(AUD, 1000 * Money::ONE,
      RiskState::Type::ACTIVE, 100 * Money::ONE, seconds(30));
    auto modification = RiskModification(risk_parameters);
    fixture.handle<LoadRiskModificationService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.SetResult(modification);
      });
    auto received_modification =
      REQUIRE_NO_THROW(fixture.m_client->load_risk_modification(id));
    TestJsonEquality(received_modification, modification);
  }

  TEST_CASE("submit_risk_modification_request") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(40, "risk_mod_account");
    auto risk_parameters = RiskParameters(CAD, 500 * Money::ONE,
      RiskState::Type::ACTIVE, 50 * Money::ONE, seconds(60));
    auto modification = RiskModification(risk_parameters);
    auto comment = Nexus::Message();
    auto request_data = AccountModificationRequest(
      41, AccountModificationRequest::Type::RISK, account,
      DirectoryEntry::MakeAccount(42, "risk_sub_account"),
      ptime(gregorian::date(2024, 5, 22)));
    fixture.handle<SubmitRiskModificationRequestService>(
      [&] (auto& request, const auto& received_account,
          const auto& received_modification, const auto& received_comment) {
        REQUIRE(received_account == account);
        TestJsonEquality(received_modification, modification);
        request.SetResult(request_data);
      });
    auto received_request = REQUIRE_NO_THROW(
      fixture.m_client->submit(account, modification, comment));
    TestJsonEquality(received_request, request_data);
  }

  TEST_CASE("load_account_modification_request_status") {
    auto fixture = Fixture();
    auto id = 43;
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::PENDING,
      DirectoryEntry::MakeAccount(44, "status_account"), 1,
      ptime(gregorian::date(2024, 5, 23)));
    fixture.handle<LoadAccountModificationRequestStatusService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.SetResult(update);
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
      DirectoryEntry::MakeAccount(46, "approver_account"), 2,
      ptime(gregorian::date(2024, 5, 24)));
    fixture.handle<ApproveAccountModificationRequestService>(
      [&] (auto& request, auto received_id, const auto& received_comment) {
        REQUIRE(received_id == id);
        request.SetResult(update);
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
      DirectoryEntry::MakeAccount(48, "rejecter_account"), 3,
      ptime(gregorian::date(2024, 5, 25)));
    fixture.handle<RejectAccountModificationRequestService>(
      [&] (auto& request, auto received_id, const auto& received_comment) {
        REQUIRE(received_id == id);
        request.SetResult(update);
      });
    auto received_update = REQUIRE_NO_THROW(
      fixture.m_client->reject_account_modification_request(id, comment));
    REQUIRE(received_update == update);
  }

  TEST_CASE("load_message") {
    auto fixture = Fixture();
    auto id = 49;
    auto message =
      Nexus::Message(id, DirectoryEntry::MakeAccount(50, "msg_account"),
        ptime(gregorian::date(2024, 5, 26)), {});
    fixture.handle<LoadMessageService>([&] (auto& request, auto received_id) {
      REQUIRE(received_id == id);
      request.SetResult(message);
    });
    auto received_message =
      REQUIRE_NO_THROW(fixture.m_client->load_message(id));
    TestJsonEquality(received_message, message);
  }

  TEST_CASE("load_message_ids") {
    auto fixture = Fixture();
    auto id = 51;
    auto ids = std::vector<Nexus::Message::Id>{52, 53, 54};
    fixture.handle<LoadMessageIdsService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == id);
        request.SetResult(ids);
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
      56, DirectoryEntry::MakeAccount(57, "sender_account"),
      ptime(gregorian::date(2024, 5, 27)), {});
    fixture.handle<SendAccountModificationRequestMessageService>(
      [&] (auto& request, auto received_id, const auto& received_message) {
        REQUIRE(received_id == id);
        request.SetResult(appended_message);
      });
    auto received_message = REQUIRE_NO_THROW(
      fixture.m_client->send_account_modification_request_message(id, message));
    TestJsonEquality(received_message, appended_message);
  }
}
