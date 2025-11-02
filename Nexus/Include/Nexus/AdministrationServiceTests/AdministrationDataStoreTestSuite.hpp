#ifndef NEXUS_ADMINISTRATION_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_ADMINISTRATION_DATA_STORE_TEST_SUITE_HPP
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"

namespace Nexus::Tests {
  TEST_CASE_TEMPLATE_DEFINE(
      "AdministrationDataStore", T, AdministrationDataStoreTestSuite) {
    using namespace Beam;
    using namespace Beam::Tests;
    using namespace boost;
    using namespace boost::posix_time;
    using namespace Nexus::DefaultCurrencies;
    auto data_store = T()();

    SUBCASE("store_and_load_identity") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto identity = AccountIdentity();
      identity.m_first_name = "John";
      identity.m_last_name = "Doe";
      data_store.with_transaction([&] {
        data_store.store(account, identity);
      });
      auto loaded_identity = data_store.with_transaction([&] {
        return data_store.load_identity(account);
      });
      test_json_equality(loaded_identity, identity);
      auto updated_identity = AccountIdentity();
      updated_identity.m_first_name = "Riley";
      updated_identity.m_last_name = "Miller";
      data_store.with_transaction([&] {
        data_store.store(account, updated_identity);
      });
      auto updated_loaded_identity = data_store.with_transaction([&] {
        return data_store.load_identity(account);
      });
      test_json_equality(updated_loaded_identity, updated_identity);
    }

    SUBCASE("load_non_existent_identity") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto identity = data_store.with_transaction([&] {
        return data_store.load_identity(account);
      });
      test_json_equality(AccountIdentity(), identity);
    }

    SUBCASE("load_all_identities") {
      auto empty_identities = data_store.with_transaction([&] {
        return data_store.load_all_account_identities();
      });
      REQUIRE(empty_identities.empty());
      auto account_a = DirectoryEntry::make_account(123, "user1");
      auto identity_a = AccountIdentity();
      identity_a.m_first_name = "Jane";
      identity_a.m_last_name = "Murphy";
      auto account_b = DirectoryEntry::make_account(345, "user2");
      auto identity_b = AccountIdentity();
      identity_b.m_first_name = "Riley";
      identity_b.m_last_name = "Miller";
      data_store.with_transaction([&] {
        data_store.store(account_a, identity_a);
        data_store.store(account_b, identity_b);
      });
      auto all_identities = data_store.with_transaction([&] {
        return data_store.load_all_account_identities();
      });
      std::sort(all_identities.begin(), all_identities.end(),
        [] (const auto& left, const auto& right) {
          return left.m_index < right.m_index;
        });
      REQUIRE(all_identities.size() == 2);
      REQUIRE(all_identities[0].m_index == account_a);
      test_json_equality(all_identities[0].m_identity, identity_a);
      REQUIRE(all_identities[1].m_index == account_b);
      test_json_equality(all_identities[1].m_identity, identity_b);
    }

    SUBCASE("store_and_load_risk_parameters") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto parameters = RiskParameters(
        CAD, Money::ONE, RiskState::Type::ACTIVE, Money::CENT, seconds(5));
      data_store.with_transaction([&] {
        data_store.store(account, parameters);
      });
      auto loaded_parameters = data_store.with_transaction([&] {
        return data_store.load_risk_parameters(account);
      });
      REQUIRE(loaded_parameters == parameters);
      auto updated_parameters = RiskParameters(USD, 10 * Money::ONE,
        RiskState::Type::DISABLED, Money::ONE, seconds(15));
      data_store.with_transaction([&] {
        data_store.store(account, updated_parameters);
      });
      auto updated_loaded_parameters = data_store.with_transaction([&] {
        return data_store.load_risk_parameters(account);
      });
      REQUIRE(updated_loaded_parameters == updated_parameters);
    }

    SUBCASE("load_non_existent_risk_parameters") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto parameters = data_store.with_transaction([&] {
        return data_store.load_risk_parameters(account);
      });
      REQUIRE(parameters == RiskParameters());
    }

    SUBCASE("load_all_risk_parameters") {
      auto empty_parameters = data_store.with_transaction([&] {
        return data_store.load_all_risk_parameters();
      });
      REQUIRE(empty_parameters.empty());
      auto account_a = DirectoryEntry::make_account(123, "user1");
      auto parameters_a = RiskParameters(USD, 100 * Money::ONE,
        RiskState::Type::ACTIVE, 10 * Money::ONE, seconds(10));
      auto account_b = DirectoryEntry::make_account(345, "user2");
      auto parameters_b = RiskParameters(EUR, 200 * Money::ONE,
        RiskState::Type::DISABLED, 20 * Money::ONE, seconds(20));
      data_store.with_transaction([&] {
        data_store.store(account_a, parameters_a);
        data_store.store(account_b, parameters_b);
      });
      auto all_parameters = data_store.with_transaction([&] {
        return data_store.load_all_risk_parameters();
      });
      std::sort(all_parameters.begin(), all_parameters.end(),
        [] (const auto& left, const auto& right) {
          return left.m_index.m_id < right.m_index.m_id;
        });
      REQUIRE(all_parameters.size() == 2);
      auto first_parameters = all_parameters[0];
      REQUIRE(first_parameters.m_index == account_a);
      REQUIRE(first_parameters.m_parameters == parameters_a);
      auto second_parameters = all_parameters[1];
      REQUIRE(second_parameters.m_index == account_b);
      REQUIRE(second_parameters.m_parameters == parameters_b);
    }

    SUBCASE("store_and_load_risk_state") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto state = RiskState(
        RiskState::Type::ACTIVE, time_from_string("2024-05-20 10:00:00"));
      data_store.with_transaction([&] {
        data_store.store(account, state);
      });
      auto loaded_state = data_store.with_transaction([&] {
        return data_store.load_risk_state(account);
      });
      REQUIRE(loaded_state == state);
      auto updated_state = RiskState(
        RiskState::Type::DISABLED, time_from_string("2024-07-05 18:00:00"));
      data_store.with_transaction([&] {
        data_store.store(account, updated_state);
      });
      auto updated_loaded_state = data_store.with_transaction([&] {
        return data_store.load_risk_state(account);
      });
      REQUIRE(updated_loaded_state == updated_state);
    }

    SUBCASE("load_non_existent_risk_state") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto state = data_store.with_transaction([&] {
        return data_store.load_risk_state(account);
      });
      REQUIRE(state == RiskState());
    }

    SUBCASE("load_all_risk_states") {
      auto empty_states = data_store.with_transaction([&] {
        return data_store.load_all_risk_states();
      });
      REQUIRE(empty_states.empty());
      auto account_a = DirectoryEntry::make_account(123, "user1");
      auto state_a = RiskState(
        RiskState::Type::CLOSE_ORDERS, time_from_string("2024-06-06 10:00:00"));
      auto account_b = DirectoryEntry::make_account(345, "user2");
      auto state_b = RiskState(
        RiskState::Type::DISABLED, time_from_string("2024-02-01 13:45:00"));
      data_store.with_transaction([&] {
        data_store.store(account_a, state_a);
        data_store.store(account_b, state_b);
      });
      auto all_states = data_store.with_transaction([&] {
        return data_store.load_all_risk_states();
      });
      std::sort(all_states.begin(), all_states.end(),
        [] (const auto& left, const auto& right) {
          return left.m_index.m_id < right.m_index.m_id;
        });
      REQUIRE(all_states.size() == 2);
      auto first_state = all_states[0];
      REQUIRE(first_state.m_index == account_a);
      REQUIRE(first_state.m_state == state_a);
      auto second_state = all_states[1];
      REQUIRE(second_state.m_index == account_b);
      REQUIRE(second_state.m_state == state_b);
    }

    SUBCASE("store_and_load_entitlement_modification") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto submission_account = DirectoryEntry::make_account(456, "admin");
      auto submission_time = time_from_string("2024-07-05 10:00:00");
      auto request = AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS, account,
        submission_account, submission_time);
      auto entitlements = std::vector<DirectoryEntry>();
      entitlements.push_back(DirectoryEntry::make_directory(23, "TSX"));
      auto modification = EntitlementModification(entitlements);
      data_store.with_transaction([&] {
        data_store.store(request, modification);
      });
      auto loaded_request = data_store.with_transaction([&] {
        return data_store.load_account_modification_request(request.get_id());
      });
      test_json_equality(loaded_request, request);
      auto loaded_modification = data_store.with_transaction([&] {
        return data_store.load_entitlement_modification(request.get_id());
      });
      test_json_equality(loaded_modification, modification);
    }

    SUBCASE("load_non_existent_entitlement_modification") {
      auto modification = data_store.with_transaction([&] {
        return data_store.load_entitlement_modification(123);
      });
      test_json_equality(modification, EntitlementModification());
    }

    SUBCASE("store_and_load_risk_modification") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto submission_account = DirectoryEntry::make_account(456, "admin");
      auto submission_time = time_from_string("2024-07-05 11:00:00");
      auto request = AccountModificationRequest(
        2, AccountModificationRequest::Type::RISK, account, submission_account,
        submission_time);
      auto parameters = RiskParameters(USD, 10000 * Money::ONE,
        RiskState::Type::ACTIVE, 1000 * Money::ONE, seconds(60));
      auto modification = RiskModification(parameters);
      data_store.with_transaction([&] {
        data_store.store(request, modification);
      });
      auto loaded_request = data_store.with_transaction([&] {
        return data_store.load_account_modification_request(request.get_id());
      });
      test_json_equality(loaded_request, request);
      auto loaded_modification = data_store.with_transaction([&] {
        return data_store.load_risk_modification(request.get_id());
      });
      test_json_equality(loaded_modification, modification);
    }

    SUBCASE("load_non_existent_risk_modification") {
      auto modification = data_store.with_transaction([&] {
        return data_store.load_risk_modification(456);
      });
      test_json_equality(modification, RiskModification());
    }

    SUBCASE("store_and_load_status") {
      auto request_id = 1;
      auto admin_account = DirectoryEntry::make_account(123, "admin");
      auto manager_account = DirectoryEntry::make_account(456, "manager");
      auto first_update = AccountModificationRequest::Update(
        AccountModificationRequest::Status::PENDING, admin_account, 1,
        time_from_string("2024-07-05 14:00:00"));
      data_store.with_transaction([&] {
        data_store.store(request_id, first_update);
      });
      auto loaded_status = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_status(request_id);
      });
      REQUIRE(loaded_status == first_update);
      auto second_update = AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED, manager_account, 2,
        time_from_string("2024-07-05 14:05:00"));
      data_store.with_transaction([&] {
        data_store.store(request_id, second_update);
      });
      loaded_status = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_status(request_id);
      });
      REQUIRE(loaded_status == second_update);
    }

    SUBCASE("load_non_existent_status") {
      auto status = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_status(999);
      });
      REQUIRE(status == AccountModificationRequest::Update());
    }

    SUBCASE("store_and_load_message") {
      auto request_id = 1;
      auto account = DirectoryEntry::make_account(123, "user1");
      auto timestamp = time_from_string("2024-07-05 15:00:00");
      auto message = Message(10, account, timestamp,
        {Message::Body::make_plain_text("Hello world")});
      data_store.with_transaction([&] {
        data_store.store(request_id, message);
      });
      auto loaded_message = data_store.with_transaction([&] {
        return data_store.load_message(message.get_id());
      });
      REQUIRE(loaded_message == message);
      auto message_ids = data_store.with_transaction([&] {
        return data_store.load_message_ids(request_id);
      });
      REQUIRE(message_ids.size() == 1);
      REQUIRE(message_ids[0] == message.get_id());
      auto last_message_id = data_store.with_transaction([&] {
        return data_store.load_last_message_id();
      });
      REQUIRE(last_message_id == message.get_id());
    }

    SUBCASE("load_messages") {
      auto last_id = data_store.with_transaction([&] {
        return data_store.load_last_message_id();
      });
      REQUIRE(last_id == 0);
      auto request_a = 1;
      auto request_b = 2;
      auto account = DirectoryEntry::make_account(123, "user1");
      auto message_a1 =
        Message(100, account, time_from_string("2024-07-05 16:00:00"), {});
      auto message_a2 =
        Message(102, account, time_from_string("2024-07-05 16:05:00"), {});
      auto message_b1 =
        Message(101, account, time_from_string("2024-07-05 16:01:00"), {});
      data_store.with_transaction([&] {
        data_store.store(request_a, message_a1);
        data_store.store(request_b, message_b1);
        data_store.store(request_a, message_a2);
      });
      auto ids_a = data_store.with_transaction([&] {
        return data_store.load_message_ids(request_a);
      });
      REQUIRE(ids_a.size() == 2);
      REQUIRE(ids_a[0] == 100);
      REQUIRE(ids_a[1] == 102);
      auto ids_b = data_store.with_transaction([&] {
        return data_store.load_message_ids(request_b);
      });
      REQUIRE(ids_b.size() == 1);
      REQUIRE(ids_b[0] == 101);
      auto ids_c = data_store.with_transaction([&] {
        return data_store.load_message_ids(3);
      });
      REQUIRE(ids_c.empty());
      last_id = data_store.with_transaction([&] {
        return data_store.load_last_message_id();
      });
      REQUIRE(last_id == 102);
      auto message_c1 =
        Message(50, account, time_from_string("2024-07-05 16:10:00"), {});
      data_store.with_transaction([&] {
        data_store.store(request_b, message_c1);
      });
      last_id = data_store.with_transaction([&] {
        return data_store.load_last_message_id();
      });
      REQUIRE(last_id == 102);
      auto loaded_message = data_store.with_transaction([&] {
        return data_store.load_message(50);
      });
      REQUIRE(loaded_message == message_c1);
      auto non_existent_message = data_store.with_transaction([&] {
        return data_store.load_message(999);
      });
      REQUIRE(non_existent_message == Message());
    }

    SUBCASE("load_account_modification_request_ids_out_of_order") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          5, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00")), modification);
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:01:00")), modification);
        data_store.store(AccountModificationRequest(
          10, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:02:00")), modification);
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:03:00")), modification);
      });
      auto ids = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_ids(0, 100);
      });
      REQUIRE(ids.size() == 4);
      REQUIRE(ids[0] == 1);
      REQUIRE(ids[1] == 3);
      REQUIRE(ids[2] == 5);
      REQUIRE(ids[3] == 10);
    }

    SUBCASE("load_account_modification_request_ids_with_start_id") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          10, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00")), modification);
        data_store.store(AccountModificationRequest(
          20, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:01:00")), modification);
        data_store.store(AccountModificationRequest(
          30, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:02:00")), modification);
        data_store.store(AccountModificationRequest(
          40, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:03:00")), modification);
      });
      auto ids = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_ids(20, 100);
      });
      REQUIRE(ids.size() == 2);
      REQUIRE(ids[0] == 30);
      REQUIRE(ids[1] == 40);
    }

    SUBCASE("load_account_modification_request_ids_with_max_count") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          10, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00")), modification);
        data_store.store(AccountModificationRequest(
          20, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:01:00")), modification);
        data_store.store(AccountModificationRequest(
          30, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:02:00")), modification);
        data_store.store(AccountModificationRequest(
          40, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:03:00")), modification);
      });
      auto ids = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_ids(0, 2);
      });
      REQUIRE(ids.size() == 2);
      REQUIRE(ids[0] == 10);
      REQUIRE(ids[1] == 20);
    }

    SUBCASE("load_account_modification_request_ids_by_account") {
      auto account_a = DirectoryEntry::make_account(100, "user_a");
      auto account_b = DirectoryEntry::make_account(200, "user_b");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-05 10:00:00")), modification);
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::ENTITLEMENTS, account_b,
          account_b, time_from_string("2024-07-05 10:01:00")), modification);
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-05 10:02:00")), modification);
      });
      auto ids = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_ids(
          account_a, 0, 100);
      });
      REQUIRE(ids.size() == 2);
      REQUIRE(ids[0] == 1);
      REQUIRE(ids[1] == 3);
    }

    SUBCASE("load_account_modification_request_ids_by_account_with_start_id") {
      auto account_a = DirectoryEntry::make_account(100, "user_a");
      auto account_b = DirectoryEntry::make_account(200, "user_b");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(1,
          AccountModificationRequest::Type::ENTITLEMENTS, account_a, account_a,
          time_from_string("2024-07-05 10:00:00")), modification);
        data_store.store(AccountModificationRequest(2,
          AccountModificationRequest::Type::ENTITLEMENTS, account_b, account_b,
          time_from_string("2024-07-05 10:01:00")), modification);
        data_store.store(AccountModificationRequest(3,
          AccountModificationRequest::Type::ENTITLEMENTS, account_a, account_a,
          time_from_string("2024-07-05 10:02:00")), modification);
        data_store.store(AccountModificationRequest(4,
          AccountModificationRequest::Type::ENTITLEMENTS, account_a, account_a,
          time_from_string("2024-07-05 10:03:00")), modification);
      });
      auto ids = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_ids(
          account_a, 1, 100);
      });
      REQUIRE(ids.size() == 2);
      REQUIRE(ids[0] == 3);
      REQUIRE(ids[1] == 4);
    }
  }
}

#endif
