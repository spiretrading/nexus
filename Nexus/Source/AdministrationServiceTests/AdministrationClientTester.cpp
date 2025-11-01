#include <future>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/TestAdministrationClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::Tests;

namespace {
  template<typename O, typename F, typename R, typename E>
  void require_operation(F&& f, const R& expected, E&& e) {
    auto operations = std::make_shared<TestAdministrationClient::Queue>();
    auto client = AdministrationClient(
      std::in_place_type<TestAdministrationClient>, operations);
    auto future = std::async(std::launch::async, [&] {
      return std::forward<F>(f)(client);
    });
    auto operation = operations->pop();
    auto specific = std::get_if<O>(&*operation);
    REQUIRE(specific);
    specific->m_result.set(expected);
    std::forward<E>(e)(std::move(future).get());
  }

  template<typename O, typename F>
  void require_operation(F&& f) {
    auto operations = std::make_shared<TestAdministrationClient::Queue>();
    auto client = AdministrationClient(
      std::in_place_type<TestAdministrationClient>, operations);
    auto future = std::async(std::launch::async, [&] {
      return std::forward<F>(f)(client);
    });
    auto operation = operations->pop();
    auto specific = std::get_if<O>(&*operation);
    REQUIRE(specific);
    specific->m_result.set();
  }

  template<typename O, typename F, typename R>
  void require_operation(F&& f, const R& expected) {
    require_operation<O>(std::forward<F>(f), expected,
      [&] (const auto& result) {
        REQUIRE(result == expected);
      });
  }

  auto get_test_risk_parameters() {
    static auto parameters = [] {
      auto parameters = RiskParameters();
      parameters.m_currency = CAD;
      parameters.m_buying_power = Money(100);
      parameters.m_allowed_state = RiskState::Type::ACTIVE;
      parameters.m_net_loss = Money(200);
      parameters.m_transition_time = seconds(100);
      return parameters;
    }();
    return parameters;
  }
}

TEST_SUITE("AdministrationClient") {
  TEST_CASE("load_accounts_by_roles") {
    auto roles = AccountRoles();
    roles.set(AccountRole::TRADER);
    auto accounts = std::vector{DirectoryEntry::make_account(1, "trader1"),
      DirectoryEntry::make_account(2, "trader2")};
    require_operation<TestAdministrationClient::LoadAccountsByRolesOperation>(
      [&] (auto& client) {
        return client.load_accounts_by_roles(roles);
      }, accounts);
  }

  TEST_CASE("load_administrators_root_entry") {
    auto entry = DirectoryEntry::make_account(1, "admin_root");
    require_operation<
      TestAdministrationClient::LoadAdministratorsRootEntryOperation>(
        [&] (auto& client) {
          return client.load_administrators_root_entry();
        }, entry);
  }

  TEST_CASE("load_services_root_entry") {
    auto entry = DirectoryEntry::make_account(2, "services_root");
    require_operation<TestAdministrationClient::LoadServicesRootEntryOperation>(
      [&] (auto& client) {
        return client.load_services_root_entry();
      }, entry);
  }

  TEST_CASE("load_trading_groups_root_entry") {
    auto entry = DirectoryEntry::make_account(3, "trading_groups_root");
    require_operation<
      TestAdministrationClient::LoadTradingGroupsRootEntryOperation>(
        [&] (auto& client) {
          return client.load_trading_groups_root_entry();
        }, entry);
  }

  TEST_CASE("check_administrator") {
    auto account = DirectoryEntry::make_account(4, "admin_account");
    require_operation<TestAdministrationClient::CheckAdministratorOperation>(
      [&] (auto& client) {
        return client.check_administrator(account);
      }, true);
  }

  TEST_CASE("load_account_roles") {
    auto account = DirectoryEntry::make_account(5, "account");
    auto roles = AccountRoles();
    roles.set(AccountRole::MANAGER);
    require_operation<TestAdministrationClient::LoadAccountRolesOperation>(
      [&] (auto& client) {
        return client.load_account_roles(account);
      }, roles);
  }

  TEST_CASE("load_parent_trading_group") {
    auto account = DirectoryEntry::make_account(6, "trader_account");
    auto parent_group = DirectoryEntry::make_account(7, "parent_group");
    require_operation<
      TestAdministrationClient::LoadParentTradingGroupOperation>(
        [&] (auto& client) {
          return client.load_parent_trading_group(account);
        }, parent_group);
  }

  TEST_CASE("load_identity") {
    auto account = DirectoryEntry::make_account(8, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "John";
    identity.m_last_name = "Doe";
    require_operation<TestAdministrationClient::LoadIdentityOperation>(
      [&] (auto& client) {
        return client.load_identity(account);
      }, identity,
      [&] (const auto& received) {
        REQUIRE(identity.m_first_name == received.m_first_name);
        REQUIRE(identity.m_last_name == received.m_last_name);
      });
  }

  TEST_CASE("store_identity") {
    auto account = DirectoryEntry::make_account(9, "identity_account");
    auto identity = AccountIdentity();
    identity.m_first_name = "Jane";
    identity.m_last_name = "Smith";
    require_operation<TestAdministrationClient::StoreIdentityOperation>(
      [&] (auto& client) {
        return client.store(account, identity);
      });
  }

  TEST_CASE("store_risk_parameters") {
    auto account = DirectoryEntry::make_account(10, "risk_account");
    auto risk_parameters = RiskParameters(USD, Money(1000),
      RiskState(RiskState::Type::ACTIVE, ptime()), Money(500), seconds(60));
    require_operation<TestAdministrationClient::StoreRiskParametersOperation>(
      [&] (auto& client) {
        client.store(account, risk_parameters);
      });
  }

  TEST_CASE("store_risk_state") {
    auto account = DirectoryEntry::make_account(11, "risk_state_account");
    auto risk_state = RiskState(RiskState::Type::DISABLED, ptime());
    require_operation<TestAdministrationClient::StoreRiskStateOperation>(
      [&] (auto& client) {
        client.store(account, risk_state);
      });
  }

  TEST_CASE("load_account_modification_request") {
    auto id = AccountModificationRequest::Id(1);
    auto request = AccountModificationRequest(
      id, AccountModificationRequest::Type::ENTITLEMENTS,
      DirectoryEntry::make_account(12, "mod_account"),
      DirectoryEntry::make_account(13, "submitter_account"), ptime());
    require_operation<
      TestAdministrationClient::LoadAccountModificationRequestOperation>(
        [&] (auto& client) {
          return client.load_account_modification_request(id);
        }, request,
        [&] (const auto& received) {
          REQUIRE(received.get_id() == id);
        });
  }

  TEST_CASE("load_account_modification_request_ids") {
    auto account = DirectoryEntry::make_account(14, "mod_ids_account");
    auto ids = std::vector<AccountModificationRequest::Id>{1, 2, 3};
    require_operation<
      TestAdministrationClient::LoadAccountModificationRequestIdsOperation>(
        [&] (auto& client) {
          return client.load_account_modification_request_ids(account, 1, 3);
        }, ids);
  }

  TEST_CASE("load_managed_account_modification_request_ids") {
    auto account = DirectoryEntry::make_account(15, "managed_mod_ids_account");
    auto ids = std::vector<AccountModificationRequest::Id>{4, 5, 6};
    require_operation<TestAdministrationClient::
      LoadManagedAccountModificationRequestIdsOperation>([&] (auto& client) {
        return client.load_managed_account_modification_request_ids(
          account, 4, 3);
      }, ids);
  }

  TEST_CASE("load_entitlement_modification") {
    auto id = AccountModificationRequest::Id(2);
    auto entitlements = std::vector<DirectoryEntry>();
    entitlements.push_back(DirectoryEntry::make_account(1, "alpha"));
    entitlements.push_back(DirectoryEntry::make_account(2, "beta"));
    auto modification = EntitlementModification(entitlements);
    require_operation<
      TestAdministrationClient::LoadEntitlementModificationOperation>(
        [&] (auto& client) {
          return client.load_entitlement_modification(id);
        }, modification,
        [&] (const auto& received) {
          REQUIRE(received.get_entitlements() == entitlements);
        });
  }

  TEST_CASE("submit_account_modification_request_entitlement") {
    auto account =
      DirectoryEntry::make_account(16, "submit_entitlement_account");
    auto modification = EntitlementModification();
    auto comment = Message(Message::Id(1), account, ptime(), {});
    auto request = AccountModificationRequest(AccountModificationRequest::Id(3),
      AccountModificationRequest::Type::ENTITLEMENTS, account, account,
      ptime());
    require_operation<
      TestAdministrationClient::SubmitEntitlementModificationRequestOperation>(
        [&] (auto& client) {
          return client.submit(account, modification, comment);
        }, request,
        [&] (const auto& received) {
          REQUIRE(received.get_id() == request.get_id());
        });
  }

  TEST_CASE("load_risk_modification") {
    auto id = AccountModificationRequest::Id(4);
    auto modification = RiskModification(get_test_risk_parameters());
    require_operation<TestAdministrationClient::LoadRiskModificationOperation>(
      [&] (auto& client) {
        return client.load_risk_modification(id);
      }, modification,
      [&] (const auto& received) {
        REQUIRE(received.get_parameters() == modification.get_parameters());
      });
  }

  TEST_CASE("submit_account_modification_request_risk") {
    auto account = DirectoryEntry::make_account(17, "submit_risk_account");
    auto modification = RiskModification(get_test_risk_parameters());
    auto comment = Message(Message::Id(2), account, ptime(), {});
    auto request = AccountModificationRequest(AccountModificationRequest::Id(5),
      AccountModificationRequest::Type::RISK, account, account, ptime());
    require_operation<
      TestAdministrationClient::SubmitRiskModificationRequestOperation>(
        [&] (auto& client) {
          return client.submit(account, modification, comment);
        }, request,
        [&] (const auto& received) {
          REQUIRE(received.get_id() == request.get_id());
        });
  }

  TEST_CASE("load_account_modification_request_status") {
    auto id = AccountModificationRequest::Id(6);
    auto status = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
        DirectoryEntry::make_account(18, "approver_account"), 1, ptime());
    require_operation<
      TestAdministrationClient::LoadAccountModificationRequestStatusOperation>(
        [&] (auto& client) {
          return client.load_account_modification_request_status(id);
        }, status);
  }

  TEST_CASE("approve_account_modification_request") {
    auto id = AccountModificationRequest::Id(7);
    auto comment = Message(Message::Id(3),
      DirectoryEntry::make_account(19, "approver_account"), ptime(), {});
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::make_account(19, "approver_account"), 2, ptime());
    require_operation<
      TestAdministrationClient::ApproveAccountModificationRequestOperation>(
        [&] (auto& client) {
          return client.approve_account_modification_request(id, comment);
        }, update);
  }

  TEST_CASE("reject_account_modification_request") {
    auto id = AccountModificationRequest::Id(8);
    auto comment = Message(Message::Id(4),
      DirectoryEntry::make_account(20, "rejector_account"), ptime(), {});
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::REJECTED,
      DirectoryEntry::make_account(20, "rejector_account"), 3, ptime());
    require_operation<
      TestAdministrationClient::RejectAccountModificationRequestOperation>(
        [&] (auto& client) {
          return client.reject_account_modification_request(id, comment);
        }, update);
  }

  TEST_CASE("load_message") {
    auto id = Message::Id(5);
    auto message = Message(
      id, DirectoryEntry::make_account(21, "message_account"), ptime(), {});
    require_operation<TestAdministrationClient::LoadMessageOperation>(
      [&] (auto& client) {
        return client.load_message(id);
      }, message);
  }

  TEST_CASE("load_message_ids") {
    auto id = AccountModificationRequest::Id(9);
    auto message_ids = std::vector<Message::Id>{6, 7, 8};
    require_operation<TestAdministrationClient::LoadMessageIdsOperation>(
      [&] (auto& client) {
        return client.load_message_ids(id);
      }, message_ids);
  }

  TEST_CASE("send_account_modification_request_message") {
    auto id = AccountModificationRequest::Id(10);
    auto message = Message(Message::Id(9),
      DirectoryEntry::make_account(22, "sender_account"), ptime(), {});
    require_operation<
      TestAdministrationClient::SendAccountModificationRequestMessageOperation>(
        [&] (auto& client) {
          return client.send_account_modification_request_message(id, message);
        }, message);
  }
}
