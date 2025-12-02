#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServiceClientFixture.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ServiceComplianceClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestServiceComplianceClient =
      ServiceComplianceClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestServiceComplianceClient> m_client;

    Fixture() {
      register_compliance_services(out(m_server.get_slots()));
      register_compliance_messages(out(m_server.get_slots()));
      m_client = make_client<TestServiceComplianceClient>();
    }

    void close_server_side() {
      auto close_token = Async<void>();
      on_request<LoadDirectoryEntryComplianceRuleEntryService>(
        [&] (auto& request, const DirectoryEntry&) {
          request.set(std::vector<ComplianceRuleEntry>());
          request.get_client().close();
          close_token.get_eval().set();
        });
      try {
        m_client->load(DirectoryEntry());
      } catch(const std::exception&) {}
      close_token.get();
    }
  };
}

TEST_SUITE("ServiceComplianceClient") {
  TEST_CASE("load") {
    auto fixture = Fixture();
    auto directory_entry = DirectoryEntry::make_account(1, "test_account");
    auto expected_entries = std::vector{ComplianceRuleEntry(
      123, directory_entry, ComplianceRuleEntry::State::ACTIVE,
      ComplianceRuleSchema())};
    fixture.on_request<LoadDirectoryEntryComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry) {
        REQUIRE(received_entry == directory_entry);
        request.set(expected_entries);
      });
    auto received_entries =
      REQUIRE_NO_THROW(fixture.m_client->load(directory_entry));
    REQUIRE(received_entries == expected_entries);
  }

  TEST_CASE("add_compliance_rule_entry") {
    auto fixture = Fixture();
    auto directory_entry = DirectoryEntry::make_account(2, "add_account");
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto state = ComplianceRuleEntry::State::ACTIVE;
    auto expected_id = ComplianceRuleEntry::Id(42);
    fixture.on_request<AddComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry, auto received_state,
          const auto& received_schema) {
        REQUIRE(received_entry == directory_entry);
        REQUIRE(received_state == state);
        REQUIRE(received_schema == schema);
        request.set(expected_id);
      });
    auto id =
      REQUIRE_NO_THROW(fixture.m_client->add(directory_entry, state, schema));
    REQUIRE(id == expected_id);
  }

  TEST_CASE("update_compliance_rule_entry") {
    auto fixture = Fixture();
    auto directory_entry = DirectoryEntry::make_account(3, "update_account");
    auto entry = ComplianceRuleEntry(100, directory_entry,
      ComplianceRuleEntry::State::ACTIVE,
      ComplianceRuleSchema("update_rule", {}));
    auto is_called = Async<bool>();
    fixture.on_request<UpdateComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry) {
        REQUIRE(received_entry == entry);
        request.set();
        is_called.get_eval().set(true);
      });
    REQUIRE_NO_THROW(fixture.m_client->update(entry));
    REQUIRE(is_called.get());
  }

  TEST_CASE("remove_compliance_rule_entry") {
    auto fixture = Fixture();
    auto expected_id = ComplianceRuleEntry::Id(77);
    auto is_called = Async<bool>();
    fixture.on_request<DeleteComplianceRuleEntryService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == expected_id);
        request.set();
        is_called.get_eval().set(true);
      });
    REQUIRE_NO_THROW(fixture.m_client->remove(expected_id));
    REQUIRE(is_called.get());
  }

  TEST_CASE("report_compliance_rule_violation") {
    auto fixture = Fixture();
    auto record = ComplianceRuleViolationRecord();
    record.m_account = DirectoryEntry::make_account(4, "report_account");
    record.m_order_id = 1234;
    record.m_rule_id = 5678;
    record.m_schema_name = "test_schema";
    record.m_reason = "violation reason";
    record.m_timestamp = time_from_string("2024-07-25 15:30:00");
    auto is_called = Async<bool>();
    fixture.on_message<ReportComplianceRuleViolationMessage>(
      [&] (auto& client, const auto& received_record) {
        REQUIRE(received_record == record);
        is_called.get_eval().set(true);
      });
    REQUIRE_NOTHROW(fixture.m_client->report(record));
    REQUIRE(is_called.get());
  }

  TEST_CASE("monitor_compliance_rule_entries") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::make_account(5, "monitor_account");
    auto initial_entries = std::vector{
      ComplianceRuleEntry(200, account, ComplianceRuleEntry::State::ACTIVE,
        ComplianceRuleSchema("ruleA", {})),
      ComplianceRuleEntry(201, account, ComplianceRuleEntry::State::DISABLED,
        ComplianceRuleSchema("ruleB", {}))
    };
    auto server_side_client =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    fixture.on_request<MonitorComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry) {
        REQUIRE(received_entry == account);
        server_side_client = &request.get_client();
        request.set(initial_entries);
      });
    auto queue = std::make_shared<Queue<ComplianceRuleEntry>>();
    auto snapshot = std::vector<ComplianceRuleEntry>();
    REQUIRE_NOTHROW(fixture.m_client->monitor_compliance_rule_entries(
      account, queue, out(snapshot)));
    REQUIRE(snapshot == initial_entries);
    auto updated_entry = ComplianceRuleEntry(202, account,
      ComplianceRuleEntry::State::ACTIVE, ComplianceRuleSchema("ruleC", {}));
    send_record_message<ComplianceRuleEntryMessage>(
      *server_side_client, updated_entry);
    REQUIRE(queue->pop() == updated_entry);
    auto reconnect_async = Async<void>();
    auto reconnected_entries = std::vector{
      ComplianceRuleEntry(200, account, ComplianceRuleEntry::State::PASSIVE,
        ComplianceRuleSchema("ruleA", {})),
      ComplianceRuleEntry(201, account, ComplianceRuleEntry::State::DISABLED,
        ComplianceRuleSchema("ruleB", {})),
      ComplianceRuleEntry(202, account, ComplianceRuleEntry::State::DISABLED,
        ComplianceRuleSchema("ruleC", {}))
    };
    fixture.on_request<MonitorComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry) {
        REQUIRE(received_entry == account);
        server_side_client = &request.get_client();
        request.set(reconnected_entries);
        reconnect_async.get_eval().set();
      });
    fixture.close_server_side();
    reconnect_async.get();
    REQUIRE(queue->pop() == reconnected_entries[0]);
    REQUIRE(queue->pop() == reconnected_entries[2]);
  }
}
