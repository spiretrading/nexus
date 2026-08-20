#ifndef NEXUS_ADMINISTRATION_DATA_STORE_TEST_SUITE_HPP
#define NEXUS_ADMINISTRATION_DATA_STORE_TEST_SUITE_HPP
#include <Beam/Queries/AndExpression.hpp>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/OrExpression.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/Queries/AccountModificationRequestAccessor.hpp"

namespace Nexus::Tests {
  TEST_CASE_TEMPLATE_DEFINE(
      "AdministrationDataStore", T, AdministrationDataStoreTestSuite) {
    using namespace Beam;
    using namespace Beam::Tests;
    using namespace boost;
    using namespace boost::posix_time;
    using namespace Nexus::Currencies;
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
        submission_account, submission_time,
        time_from_string("2024-08-01 00:00:00"));
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
        submission_time, time_from_string("2024-08-15 00:00:00"));
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

    SUBCASE("load_account_modification_request_updates") {
      auto request_id = 1;
      auto admin_account = DirectoryEntry::make_account(123, "admin");
      auto manager_account = DirectoryEntry::make_account(456, "manager");
      auto first_update = AccountModificationRequest::Update(
        AccountModificationRequest::Status::PENDING, admin_account, 1,
        time_from_string("2024-07-05 14:00:00"));
      auto second_update = AccountModificationRequest::Update(
        AccountModificationRequest::Status::GRANTED, manager_account, 2,
        time_from_string("2024-07-05 14:05:00"));
      data_store.with_transaction([&] {
        data_store.store(request_id, first_update);
        data_store.store(request_id, second_update);
      });
      auto updates = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_updates(request_id);
      });
      REQUIRE(updates.size() == 2);
      REQUIRE(updates[0] == first_update);
      REQUIRE(updates[1] == second_update);
    }

    SUBCASE("load_non_existent_updates") {
      auto updates = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_updates(999);
      });
      REQUIRE(updates.empty());
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

    SUBCASE("load_account_modification_requests_head_and_tail") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {5, 1, 10, 3}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_head(3));
      auto head = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(head.size() == 3);
      REQUIRE(head[0].get_id() == 1);
      REQUIRE(head[1].get_id() == 3);
      REQUIRE(head[2].get_id() == 5);
      REQUIRE(head[0].get_account() == account);
      query.set_snapshot_limit(SnapshotLimit::from_tail(2));
      auto tail = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(tail.size() == 2);
      REQUIRE(tail[0].get_id() == 5);
      REQUIRE(tail[1].get_id() == 10);
    }

    SUBCASE("load_account_modification_requests_with_offset") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {1, 2, 3, 4, 5}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_tail(2));
      auto first = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(first.size() == 2);
      REQUIRE(first[0].get_id() == 4);
      REQUIRE(first[1].get_id() == 5);
      query.set_offset(2);
      auto second = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(second.size() == 2);
      REQUIRE(second[0].get_id() == 2);
      REQUIRE(second[1].get_id() == 3);
      query.set_snapshot_limit(SnapshotLimit::from_head(2));
      auto head = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(head.size() == 2);
      REQUIRE(head[0].get_id() == 3);
      REQUIRE(head[1].get_id() == 4);
      query.set_offset(100);
      auto beyond = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(beyond.empty());
    }

    SUBCASE("load_account_modification_requests_sorted_by_last_update") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {1, 2, 3}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
        data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-09 10:00:00")));
        data_store.store(2, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-07 10:00:00")));
        data_store.store(3, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-08 10:00:00")));
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::LAST_UPDATED);
      query.set_snapshot_limit(SnapshotLimit::from_tail(3));
      auto requests = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(requests.size() == 3);
      REQUIRE(requests[0].get_id() == 2);
      REQUIRE(requests[1].get_id() == 3);
      REQUIRE(requests[2].get_id() == 1);
      query.set_snapshot_limit(SnapshotLimit::from_tail(2));
      auto page = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(page.size() == 2);
      REQUIRE(page[0].get_id() == 3);
      REQUIRE(page[1].get_id() == 1);
      query.set_anchor(AccountModificationRequestAnchor(
        3, time_from_string("2024-07-08 10:00:00"), ""));
      auto next = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(next.size() == 1);
      REQUIRE(next[0].get_id() == 2);
      query.set_anchor(optional<AccountModificationRequestAnchor>());
      query.set_offset(1);
      auto skipped = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(skipped.size() == 2);
      REQUIRE(skipped[0].get_id() == 2);
      REQUIRE(skipped[1].get_id() == 3);
      query.set_offset(0);
      query.set_sort_field(AccountModificationRequestQuery::SortField::CREATED);
      query.set_snapshot_limit(SnapshotLimit::from_tail(3));
      auto created = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(created.size() == 3);
      REQUIRE(created[0].get_id() == 1);
      REQUIRE(created[1].get_id() == 2);
      REQUIRE(created[2].get_id() == 3);
    }

    SUBCASE("load_account_modification_requests_sorted_by_effective_date") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      auto dates = std::vector<std::string>{
        "2024-09-03 00:00:00", "2024-09-01 00:00:00", "2024-09-02 00:00:00"};
      data_store.with_transaction([&] {
        for(auto i = 0; i != 3; ++i) {
          data_store.store(AccountModificationRequest(
            i + 1, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string(dates[i])), modification);
        }
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::EFFECTIVE_DATE);
      query.set_snapshot_limit(SnapshotLimit::from_tail(3));
      auto requests = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(requests.size() == 3);
      REQUIRE(requests[0].get_id() == 2);
      REQUIRE(requests[1].get_id() == 3);
      REQUIRE(requests[2].get_id() == 1);
      query.set_snapshot_limit(SnapshotLimit::from_tail(2));
      query.set_anchor(AccountModificationRequestAnchor(
        3, time_from_string("2024-09-02 00:00:00"), ""));
      auto next = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(next.size() == 1);
      REQUIRE(next[0].get_id() == 2);
      query.set_anchor(optional<AccountModificationRequestAnchor>());
      query.set_snapshot_limit(SnapshotLimit::from_head(2));
      auto head = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(head.size() == 2);
      REQUIRE(head[0].get_id() == 2);
      REQUIRE(head[1].get_id() == 3);
    }

    SUBCASE("load_account_modification_requests_with_a_missing_anchor") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {1, 2, 3}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
        data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-09 10:00:00")));
        data_store.store(2, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-07 10:00:00")));
        data_store.store(3, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-08 10:00:00")));
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_head(10));
      query.set_anchor(AccountModificationRequestAnchor(
        999, time_from_string("2024-07-01 10:00:00"), ""));
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::LAST_UPDATED);
      auto by_update = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(by_update.size() == 3);
      REQUIRE(by_update[0].get_id() == 2);
      REQUIRE(by_update[1].get_id() == 3);
      REQUIRE(by_update[2].get_id() == 1);
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::EFFECTIVE_DATE);
      auto by_effective_date = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(by_effective_date.size() == 3);
      query.set_snapshot_limit(SnapshotLimit::from_tail(10));
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::LAST_UPDATED);
      query.set_anchor(AccountModificationRequestAnchor(999,
        time_from_string("2024-07-20 10:00:00"), ""));
      auto tail = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(tail.size() == 3);
      REQUIRE(tail[0].get_id() == 2);
      REQUIRE(tail[1].get_id() == 3);
      REQUIRE(tail[2].get_id() == 1);
      query.set_anchor(AccountModificationRequestAnchor(
        999, time_from_string("2024-07-01 10:00:00"), ""));
      auto before_everything = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(before_everything.empty());
    }

    SUBCASE("load_account_modification_requests_filtered") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto other = DirectoryEntry::make_account(101, "user_b");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::RISK, account, account,
          time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, other, other,
          time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          4, AccountModificationRequest::Type::ENTITLEMENTS, other, other,
          time_from_string("2024-07-06 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-07 10:00:00")));
        data_store.store(2, AccountModificationRequest::Update(
          AccountModificationRequest::Status::PENDING, account, 1,
          time_from_string("2024-07-08 10:00:00")));
        data_store.store(3, AccountModificationRequest::Update(
          AccountModificationRequest::Status::REJECTED, other, 1,
          time_from_string("2024-07-09 10:00:00")));
      });
      auto load = [&] (const AccountModificationRequestQuery& query) {
        return data_store.with_transaction([&] {
          return data_store.load_account_modification_requests(query);
        });
      };
      auto accessor = AccountModificationRequestAccessor::from_parameter(0);
      auto query = AccountModificationRequestQuery();
      query.set_snapshot_limit(SnapshotLimit::from_head(10));
      query.set_filter(accessor.get_type() ==
        static_cast<int>(AccountModificationRequest::Type::RISK));
      auto by_category = load(query);
      REQUIRE(by_category.size() == 1);
      REQUIRE(by_category[0].get_id() == 2);
      query.set_filter(OrExpression(
        accessor.get_status() ==
          static_cast<int>(AccountModificationRequest::Status::GRANTED),
        accessor.get_status() ==
          static_cast<int>(AccountModificationRequest::Status::REJECTED)));
      auto by_status = load(query);
      REQUIRE(by_status.size() == 2);
      REQUIRE(by_status[0].get_id() == 1);
      REQUIRE(by_status[1].get_id() == 3);
      query.set_filter(accessor.get_last_update_timestamp() >=
        time_from_string("2024-07-08 00:00:00"));
      auto from_start = load(query);
      REQUIRE(from_start.size() == 2);
      REQUIRE(from_start[0].get_id() == 2);
      REQUIRE(from_start[1].get_id() == 3);
      query.set_filter(AndExpression(
        accessor.get_last_update_timestamp() >=
          time_from_string("2024-07-08 00:00:00"),
        accessor.get_last_update_timestamp() <=
          time_from_string("2024-07-08 23:59:59")));
      auto within_range = load(query);
      REQUIRE(within_range.size() == 1);
      REQUIRE(within_range[0].get_id() == 2);
      query.set_filter(accessor.get_account() != static_cast<int>(other.m_id));
      auto excluded = load(query);
      REQUIRE(excluded.size() == 2);
      REQUIRE(excluded[0].get_id() == 1);
      REQUIRE(excluded[1].get_id() == 2);
      query.set_filter(accessor.get_status() ==
        static_cast<int>(AccountModificationRequest::Status::NONE));
      auto without_updates = load(query);
      REQUIRE(without_updates.size() == 1);
      REQUIRE(without_updates[0].get_id() == 4);
      query.set_filter(accessor.get_last_update_timestamp() ==
        time_from_string("2024-07-06 10:00:00"));
      auto without_updates_timestamp = load(query);
      REQUIRE(without_updates_timestamp.size() == 1);
      REQUIRE(without_updates_timestamp[0].get_id() == 4);
    }

    SUBCASE("load_account_modification_requests_anchored_by_name") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {1, 2, 3, 4}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_sort_field(AccountModificationRequestQuery::SortField::ACCOUNT);
      query.set_snapshot_limit(SnapshotLimit::from_head(10));
      query.set_anchor(AccountModificationRequestAnchor(
        2, time_from_string("2024-07-05 10:00:00"), "user_a"));
      auto head = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(head.size() == 2);
      REQUIRE(head[0].get_id() == 3);
      REQUIRE(head[1].get_id() == 4);
      query.set_snapshot_limit(SnapshotLimit::from_tail(10));
      auto tail = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(tail.size() == 1);
      REQUIRE(tail[0].get_id() == 1);
    }

    SUBCASE("load_account_modification_request_counts") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto other = DirectoryEntry::make_account(101, "user_b");
      auto modification = EntitlementModification();
      auto store = [&] (AccountModificationRequest::Id id,
          AccountModificationRequest::Type type, const DirectoryEntry& owner) {
        data_store.store(AccountModificationRequest(
          id, type, owner, owner, time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      };
      auto update = [&] (AccountModificationRequest::Id id,
          AccountModificationRequest::Status status) {
        data_store.store(id, AccountModificationRequest::Update(
          status, DirectoryEntry::make_account(100, "user_a"), 1,
          time_from_string("2024-07-07 10:00:00")));
      };
      data_store.with_transaction([&] {
        store(1, AccountModificationRequest::Type::ENTITLEMENTS, account);
        store(2, AccountModificationRequest::Type::RISK, account);
        store(3, AccountModificationRequest::Type::ENTITLEMENTS, account);
        store(4, AccountModificationRequest::Type::ENTITLEMENTS, other);
        store(5, AccountModificationRequest::Type::ENTITLEMENTS, account);
        update(1, AccountModificationRequest::Status::GRANTED);
        update(2, AccountModificationRequest::Status::REJECTED);
        update(3, AccountModificationRequest::Status::PENDING);
        update(4, AccountModificationRequest::Status::SCHEDULED);
      });
      auto query = AccountModificationRequestQuery();
      auto counts = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_counts(query);
      });
      REQUIRE(counts.m_granted == 1);
      REQUIRE(counts.m_rejected == 1);
      REQUIRE(counts.m_pending == 3);
      auto accounts = std::vector<DirectoryEntry>{account};
      auto restricted = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_counts(
          accounts, query);
      });
      REQUIRE(restricted.m_granted == 1);
      REQUIRE(restricted.m_rejected == 1);
      REQUIRE(restricted.m_pending == 2);
      query.set_filter(
        AccountModificationRequestAccessor::from_parameter(0).get_type() ==
          static_cast<int>(AccountModificationRequest::Type::RISK));
      auto by_category = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_counts(query);
      });
      REQUIRE(by_category.m_granted == 0);
      REQUIRE(by_category.m_rejected == 1);
      REQUIRE(by_category.m_pending == 0);
    }

    SUBCASE("load_account_modification_requests_with_anchor") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {10, 20, 30, 40}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      query.set_anchor(
        AccountModificationRequestAnchor(20, not_a_date_time, ""));
      auto head = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(head.size() == 2);
      REQUIRE(head[0].get_id() == 30);
      REQUIRE(head[1].get_id() == 40);
      query.set_snapshot_limit(SnapshotLimit::from_tail(100));
      query.set_anchor(
        AccountModificationRequestAnchor(30, not_a_date_time, ""));
      auto tail = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(tail.size() == 2);
      REQUIRE(tail[0].get_id() == 10);
      REQUIRE(tail[1].get_id() == 20);
    }

    SUBCASE("load_account_modification_requests_from_a_default_anchor") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {10, 20, 30}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      query.set_sort_field(
        AccountModificationRequestQuery::SortField::EFFECTIVE_DATE);
      query.set_anchor(AccountModificationRequestAnchor());
      auto requests = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(requests.size() == 3);
      REQUIRE(requests[0].get_id() == 10);
      REQUIRE(requests[1].get_id() == 20);
      REQUIRE(requests[2].get_id() == 30);
    }

    SUBCASE("load_account_modification_requests_by_accounts") {
      auto account_a = DirectoryEntry::make_account(100, "user_a");
      auto account_b = DirectoryEntry::make_account(200, "user_b");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::ENTITLEMENTS, account_b,
          account_b, time_from_string("2024-07-05 10:01:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-05 10:02:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account_a);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      auto restricted = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(
          {account_a}, query);
      });
      REQUIRE(restricted.size() == 2);
      REQUIRE(restricted[0].get_id() == 1);
      REQUIRE(restricted[1].get_id() == 3);
      auto unrestricted = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(unrestricted.size() == 3);
      auto empty = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests({}, query);
      });
      REQUIRE(empty.empty());
    }

    SUBCASE("load_account_modification_requests_accounts_bound_results") {
      auto account_a = DirectoryEntry::make_account(100, "user_a");
      auto account_b = DirectoryEntry::make_account(200, "user_b");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::ENTITLEMENTS, account_b,
          account_b, time_from_string("2024-07-05 10:01:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      });
      auto query = AccountModificationRequestQuery();
      query.set_index(account_b);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      auto requests = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(
          {account_a}, query);
      });
      REQUIRE(requests.size() == 1);
      REQUIRE(requests[0].get_id() == 1);
      query.set_anchor(
        AccountModificationRequestAnchor(1, not_a_date_time, ""));
      auto anchored = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(
          {account_a}, query);
      });
      REQUIRE(anchored.empty());
      auto query2 = AccountModificationRequestQuery();
      query2.set_index(account_a);
      query2.set_snapshot_limit(SnapshotLimit::from_head(100));
      query2.set_filter(ConstantExpression(false));
      auto filtered = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(
          {account_a}, query2);
      });
      REQUIRE(filtered.empty());
    }

    SUBCASE("load_account_modification_requests_by_category") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), EntitlementModification());
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::RISK, account, account,
          time_from_string("2024-07-06 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), RiskModification());
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-07 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), EntitlementModification());
      });
      auto request = AccountModificationRequestAccessor::from_parameter(0);
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      query.set_filter(request.get_type() == ConstantExpression(
        static_cast<int>(AccountModificationRequest::Type::ENTITLEMENTS)));
      auto entitlements = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(entitlements.size() == 2);
      REQUIRE(entitlements[0].get_id() == 1);
      REQUIRE(entitlements[1].get_id() == 3);
      query.set_filter(request.get_type() == ConstantExpression(
        static_cast<int>(AccountModificationRequest::Type::RISK)));
      auto risk = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(risk.size() == 1);
      REQUIRE(risk[0].get_id() == 2);
    }

    SUBCASE("load_account_modification_requests_by_timestamp") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-10 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-15 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
      });
      auto request = AccountModificationRequestAccessor::from_parameter(0);
      auto query = AccountModificationRequestQuery();
      query.set_index(account);
      query.set_snapshot_limit(SnapshotLimit::from_head(100));
      query.set_filter(request.get_timestamp() >=
        ConstantExpression(time_from_string("2024-07-10 00:00:00")));
      auto recent = data_store.with_transaction([&] {
        return data_store.load_account_modification_requests(query);
      });
      REQUIRE(recent.size() == 2);
      REQUIRE(recent[0].get_id() == 2);
      REQUIRE(recent[1].get_id() == 3);
    }

    SUBCASE("load_batched_statuses_and_message_counts") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        for(auto id : {1, 2, 3}) {
          data_store.store(AccountModificationRequest(
            id, AccountModificationRequest::Type::ENTITLEMENTS, account,
            account, time_from_string("2024-07-05 10:00:00"),
            time_from_string("2024-08-01 00:00:00")), modification);
        }
        data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::PENDING, account, 0,
          time_from_string("2024-07-05 10:00:00")));
        data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account, 1,
          time_from_string("2024-07-05 11:00:00")));
        data_store.store(3, AccountModificationRequest::Update(
          AccountModificationRequest::Status::REJECTED, account, 0,
          time_from_string("2024-07-05 12:00:00")));
        data_store.store(
          1, Message(10, account, time_from_string("2024-07-05 13:00:00"), {}));
        data_store.store(
          1, Message(11, account, time_from_string("2024-07-05 14:00:00"), {}));
      });
      auto ids = std::vector<AccountModificationRequest::Id>({1, 2, 3});
      auto statuses = data_store.with_transaction([&] {
        return data_store.load_account_modification_request_statuses(ids);
      });
      REQUIRE(statuses.size() == 3);
      REQUIRE(statuses[0].m_status ==
        AccountModificationRequest::Status::GRANTED);
      REQUIRE(statuses[1].m_status == AccountModificationRequest::Status::NONE);
      REQUIRE(statuses[2].m_status ==
        AccountModificationRequest::Status::REJECTED);
      auto counts = data_store.with_transaction([&] {
        return data_store.load_message_counts(ids);
      });
      REQUIRE(counts.size() == 3);
      REQUIRE(counts[0] == 2);
      REQUIRE(counts[1] == 0);
      REQUIRE(counts[2] == 0);
    }

    SUBCASE("load_batches_with_no_ids") {
      auto ids = std::vector<AccountModificationRequest::Id>();
      data_store.with_transaction([&] {
        REQUIRE(
          data_store.load_account_modification_request_statuses(ids).empty());
        REQUIRE(data_store.load_message_counts(ids).empty());
        REQUIRE(data_store.load_previous_granted_requests(ids).empty());
        REQUIRE(data_store.load_entitlement_modifications(ids).empty());
        REQUIRE(data_store.load_risk_modifications(ids).empty());
      });
    }

    SUBCASE("load_previous_granted_requests") {
      auto account_a = DirectoryEntry::make_account(100, "user_a");
      auto account_b = DirectoryEntry::make_account(200, "user_b");
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-06 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          3, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-07 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          4, AccountModificationRequest::Type::ENTITLEMENTS, account_b,
          account_b, time_from_string("2024-07-08 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          5, AccountModificationRequest::Type::ENTITLEMENTS, account_b,
          account_b, time_from_string("2024-07-09 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(1, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account_a, 0,
          time_from_string("2024-07-05 11:00:00")));
        data_store.store(2, AccountModificationRequest::Update(
          AccountModificationRequest::Status::REJECTED, account_a, 0,
          time_from_string("2024-07-06 11:00:00")));
        data_store.store(4, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account_b, 0,
          time_from_string("2024-07-08 11:00:00")));
        data_store.store(AccountModificationRequest(
          6, AccountModificationRequest::Type::RISK, account_a, account_a,
          time_from_string("2024-07-10 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(AccountModificationRequest(
          7, AccountModificationRequest::Type::ENTITLEMENTS, account_a,
          account_a, time_from_string("2024-07-11 10:00:00"),
          time_from_string("2024-08-01 00:00:00")), modification);
        data_store.store(6, AccountModificationRequest::Update(
          AccountModificationRequest::Status::GRANTED, account_a, 0,
          time_from_string("2024-07-10 11:00:00")));
      });
      auto ids = std::vector<AccountModificationRequest::Id>({1, 3, 5, 7});
      auto predecessors = data_store.with_transaction([&] {
        return data_store.load_previous_granted_requests(ids);
      });
      REQUIRE(predecessors.size() == 4);
      REQUIRE(!predecessors[0]);
      REQUIRE(predecessors[1] == 1);
      REQUIRE(predecessors[2] == 4);
      REQUIRE(predecessors[3] == 1);
    }

    SUBCASE("load_batched_modifications") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto entitlement = DirectoryEntry::make_directory(23, "TSX");
      auto parameters = RiskParameters(
        USD, 100 * Money::ONE, RiskState::Type::ACTIVE, Money::ONE, seconds(5));
      data_store.with_transaction([&] {
        data_store.store(AccountModificationRequest(
          1, AccountModificationRequest::Type::ENTITLEMENTS, account, account,
          time_from_string("2024-07-05 10:00:00"),
          time_from_string("2024-08-01 00:00:00")),
          EntitlementModification({entitlement}));
        data_store.store(AccountModificationRequest(
          2, AccountModificationRequest::Type::RISK, account, account,
          time_from_string("2024-07-06 10:00:00"),
          time_from_string("2024-08-01 00:00:00")),
          RiskModification(parameters));
      });
      auto entitlements = data_store.with_transaction([&] {
        return data_store.load_entitlement_modifications({1, 2});
      });
      REQUIRE(entitlements.size() == 2);
      REQUIRE(entitlements[0].get_entitlements().size() == 1);
      REQUIRE(entitlements[0].get_entitlements()[0] == entitlement);
      REQUIRE(entitlements[1].get_entitlements().empty());
      auto risks = data_store.with_transaction([&] {
        return data_store.load_risk_modifications({2, 1});
      });
      REQUIRE(risks.size() == 2);
      REQUIRE(risks[0].get_parameters() == parameters);
      REQUIRE(risks[1].get_parameters() == RiskParameters());
    }

    SUBCASE("store_and_load_effective_date") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto submission_account = DirectoryEntry::make_account(456, "admin");
      auto submission_time = time_from_string("2024-07-05 10:00:00");
      auto effective_date = time_from_string("2024-09-01 00:00:00");
      auto request = AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS, account,
        submission_account, submission_time, effective_date);
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(request, modification);
      });
      auto loaded_request = data_store.with_transaction([&] {
        return data_store.load_account_modification_request(request.get_id());
      });
      test_json_equality(loaded_request, request);
      REQUIRE(loaded_request.get_effective_date() == effective_date);
    }

    SUBCASE("store_and_load_notifications") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto notification_a = Notification("aaa-001", account,
        "First notification.", "", Notification::Category::ACCOUNT_MODIFICATION,
        time_from_string("2026-04-21 10:00:00"), false);
      auto notification_b = Notification("aaa-002", account,
        "Second notification.", "", Notification::Category::REPORT,
        time_from_string("2026-04-21 11:00:00"), true);
      auto notification_c = Notification("aaa-003", account,
        "Third notification.", "", Notification::Category::ACCOUNT_MODIFICATION,
        time_from_string("2026-04-21 12:00:00"), false);
      data_store.with_transaction([&] {
        data_store.store(notification_a);
        data_store.store(notification_b);
        data_store.store(notification_c);
      });
      auto all = data_store.with_transaction([&] {
        return data_store.load_notifications(
          account, "", SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
      });
      REQUIRE(all.size() == 3);
      REQUIRE(all[0].m_id == "aaa-001");
      REQUIRE(all[1].m_id == "aaa-002");
      REQUIRE(all[2].m_id == "aaa-003");
    }

    SUBCASE("load_notifications_unread_only") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "bbb-001", account, "Unread.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "bbb-002", account, "Read.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), true));
        data_store.store(Notification(
          "bbb-003", account, "Unread.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 12:00:00"), false));
      });
      auto unread = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::UNREAD);
      });
      REQUIRE(unread.size() == 2);
      REQUIRE(unread[0].m_id == "bbb-001");
      REQUIRE(unread[1].m_id == "bbb-003");
    }

    SUBCASE("load_notifications_read_only") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "ccc-001", account, "Unread.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "ccc-002", account, "Read.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), true));
      });
      auto read = data_store.with_transaction([&] {
        return data_store.load_notifications(
          account, "", SnapshotLimit::UNLIMITED, Notification::ReadState::READ);
      });
      REQUIRE(read.size() == 1);
      REQUIRE(read[0].m_id == "ccc-002");
    }

    SUBCASE("load_notifications_tail_limit") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "ddd-001", account, "First.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "ddd-002", account, "Second.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
        data_store.store(Notification(
          "ddd-003", account, "Third.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 12:00:00"), false));
      });
      auto tail = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::from_tail(2), Notification::ReadState::ALL);
      });
      REQUIRE(tail.size() == 2);
      REQUIRE(tail[0].m_id == "ddd-002");
      REQUIRE(tail[1].m_id == "ddd-003");
    }

    SUBCASE("load_notifications_head_limit") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "eee-001", account, "First.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "eee-002", account, "Second.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
        data_store.store(Notification(
          "eee-003", account, "Third.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 12:00:00"), false));
      });
      auto head = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::from_head(2), Notification::ReadState::ALL);
      });
      REQUIRE(head.size() == 2);
      REQUIRE(head[0].m_id == "eee-001");
      REQUIRE(head[1].m_id == "eee-002");
    }

    SUBCASE("load_notifications_different_accounts") {
      auto account_a = DirectoryEntry::make_account(100, "user_a");
      auto account_b = DirectoryEntry::make_account(200, "user_b");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "fff-001", account_a, "For A.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "fff-002", account_b, "For B.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
      });
      auto a_notifications = data_store.with_transaction([&] {
        return data_store.load_notifications(account_a, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
      });
      REQUIRE(a_notifications.size() == 1);
      REQUIRE(a_notifications[0].m_id == "fff-001");
      auto b_notifications = data_store.with_transaction([&] {
        return data_store.load_notifications(account_b, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
      });
      REQUIRE(b_notifications.size() == 1);
      REQUIRE(b_notifications[0].m_id == "fff-002");
    }

    SUBCASE("load_notifications_empty") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      auto notifications = data_store.with_transaction([&] {
        return data_store.load_notifications(
          account, "", SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
      });
      REQUIRE(notifications.empty());
    }

    SUBCASE("mark_notification_as_read") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "ggg-001", account, "Unread.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification("ggg-002", account, "Also unread.", "",
          Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
      });
      data_store.with_transaction([&] {
        data_store.mark_notification_as_read("ggg-001");
      });
      auto all = data_store.with_transaction([&] {
        return data_store.load_notifications(
          account, "", SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
      });
      REQUIRE(all.size() == 2);
      REQUIRE(all[0].m_id == "ggg-001");
      REQUIRE(all[0].m_is_read);
      REQUIRE(all[1].m_id == "ggg-002");
      REQUIRE(!all[1].m_is_read);
    }

    SUBCASE("mark_notification_as_read_filters") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "hhh-001", account, "First.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "hhh-002", account, "Second.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
      });
      data_store.with_transaction([&] {
        data_store.mark_notification_as_read("hhh-001");
      });
      auto unread = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::UNREAD);
      });
      REQUIRE(unread.size() == 1);
      REQUIRE(unread[0].m_id == "hhh-002");
      auto read = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::READ);
      });
      REQUIRE(read.size() == 1);
      REQUIRE(read[0].m_id == "hhh-001");
    }

    SUBCASE("mark_notification_as_unread") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "jjj-001", account, "Read.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), true));
        data_store.store(Notification(
          "jjj-002", account, "Also read.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), true));
      });
      data_store.with_transaction([&] {
        data_store.mark_notification_as_unread("jjj-001");
      });
      auto all = data_store.with_transaction([&] {
        return data_store.load_notifications(
          account, "", SnapshotLimit::UNLIMITED, Notification::ReadState::ALL);
      });
      REQUIRE(all.size() == 2);
      REQUIRE(all[0].m_id == "jjj-001");
      REQUIRE(!all[0].m_is_read);
      REQUIRE(all[1].m_id == "jjj-002");
      REQUIRE(all[1].m_is_read);
    }

    SUBCASE("mark_notification_as_unread_filters") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "kkk-001", account, "First.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), true));
        data_store.store(Notification(
          "kkk-002", account, "Second.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), true));
      });
      data_store.with_transaction([&] {
        data_store.mark_notification_as_unread("kkk-001");
      });
      auto unread = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::UNREAD);
      });
      REQUIRE(unread.size() == 1);
      REQUIRE(unread[0].m_id == "kkk-001");
      auto read = data_store.with_transaction([&] {
        return data_store.load_notifications(account, "",
          SnapshotLimit::UNLIMITED, Notification::ReadState::READ);
      });
      REQUIRE(read.size() == 1);
      REQUIRE(read[0].m_id == "kkk-002");
    }

    SUBCASE("load_non_existent_request") {
      REQUIRE_THROWS_AS(data_store.with_transaction([&] {
        return data_store.load_account_modification_request(42);
      }), AdministrationDataStoreException);
    }

    SUBCASE("load_notifications_from_an_absent_anchor") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "aaa-001", account, "First.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "ccc-003", account, "Third.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
        data_store.store(Notification(
          "eee-005", account, "Fifth.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 12:00:00"), false));
      });
      auto load = [&] (const Notification::Id& id, SnapshotLimit limit) {
        return data_store.with_transaction([&] {
          return data_store.load_notifications(
            account, id, limit, Notification::ReadState::ALL);
        });
      };
      auto head = load("bbb-002", SnapshotLimit::from_head(10));
      REQUIRE(head.size() == 2);
      REQUIRE(head[0].m_id == "ccc-003");
      REQUIRE(head[1].m_id == "eee-005");
      auto tail = load("ddd-004", SnapshotLimit::from_tail(10));
      REQUIRE(tail.size() == 2);
      REQUIRE(tail[0].m_id == "aaa-001");
      REQUIRE(tail[1].m_id == "ccc-003");
      REQUIRE(load("zzz-999", SnapshotLimit::from_head(10)).empty());
      REQUIRE(load("aaa-000", SnapshotLimit::from_tail(10)).empty());
    }

    SUBCASE("load_notification_helper") {
      auto account = DirectoryEntry::make_account(100, "user_a");
      data_store.with_transaction([&] {
        data_store.store(Notification(
          "iii-001", account, "First.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 10:00:00"), false));
        data_store.store(Notification(
          "iii-002", account, "Second.", "", Notification::Category::REPORT,
          time_from_string("2026-04-21 11:00:00"), false));
      });
      auto found = data_store.with_transaction([&] {
        return load_notification(data_store, account, "iii-001");
      });
      REQUIRE(found);
      REQUIRE(found->m_id == "iii-001");
      REQUIRE(found->m_description == "First.");
      auto not_found = data_store.with_transaction([&] {
        return load_notification(data_store, account, "nonexistent");
      });
      REQUIRE(!not_found);
      auto wrong_account = DirectoryEntry::make_account(200, "user_b");
      auto wrong = data_store.with_transaction([&] {
        return load_notification(data_store, wrong_account, "iii-001");
      });
      REQUIRE(!wrong);
    }

    SUBCASE("update_request_effective_date") {
      auto account = DirectoryEntry::make_account(123, "user1");
      auto submission_account = DirectoryEntry::make_account(456, "admin");
      auto submission_time = time_from_string("2024-07-05 10:00:00");
      auto effective_date = time_from_string("2024-09-01 00:00:00");
      auto request = AccountModificationRequest(
        1, AccountModificationRequest::Type::ENTITLEMENTS, account,
        submission_account, submission_time, effective_date);
      auto modification = EntitlementModification();
      data_store.with_transaction([&] {
        data_store.store(request, modification);
      });
      auto updated_effective_date = time_from_string("2024-10-15 00:00:00");
      data_store.with_transaction([&] {
        data_store.store_effective_date(
          request.get_id(), updated_effective_date);
      });
      auto loaded_request = data_store.with_transaction([&] {
        return data_store.load_account_modification_request(request.get_id());
      });
      REQUIRE(loaded_request.get_effective_date() == updated_effective_date);
      REQUIRE(loaded_request.get_id() == request.get_id());
      REQUIRE(loaded_request.get_type() == request.get_type());
      REQUIRE(loaded_request.get_account() == request.get_account());
      REQUIRE(loaded_request.get_submission_account() ==
        request.get_submission_account());
      REQUIRE(loaded_request.get_timestamp() == request.get_timestamp());
    }
  }
}

#endif
