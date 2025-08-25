#include <memory>
#include <Beam/Queues/Queue.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/SignalHandling/NullSlot.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/Compliance/ServiceComplianceClient.hpp"

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
using namespace Nexus;

namespace {
  struct Fixture {
    using TestServiceComplianceClient =
      ServiceComplianceClient<TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unique_ptr<TestServiceComplianceClient> m_client;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    Fixture()
        : m_server_connection(std::make_shared<TestServerConnection>()),
          m_server(m_server_connection,
            factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot()) {
      RegisterComplianceServices(Store(m_server.GetSlots()));
      RegisterComplianceMessages(Store(m_server.GetSlots()));
      auto builder = TestServiceProtocolClientBuilder([=, this] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_server_connection);
      }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_client = std::make_unique<TestServiceComplianceClient>(builder);
    }

    template<typename T, typename F>
    void on_request(F&& handler) {
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

    template<typename T, typename F>
    void on_message(F&& handler) {
      using Slot = typename Beam::Services::Details::RecordMessageSlot<
        RecordMessage<
          T, TestServiceProtocolServer::ServiceProtocolClient>>::Slot;
      auto& stored_handler = m_handlers[typeid(T)];
      if(stored_handler) {
        *std::static_pointer_cast<Slot>(stored_handler) =
          std::forward<F>(handler);
      } else {
        auto shared_handler = std::make_shared<Slot>(std::forward<F>(handler));
        stored_handler = shared_handler;
        AddMessageSlot<T>(Store(m_server.GetSlots()),
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

TEST_SUITE("ServiceComplianceClient") {
  TEST_CASE("load") {
    auto fixture = Fixture();
    auto directory_entry = DirectoryEntry::MakeAccount(1, "test_account");
    auto expected_entries = std::vector{ComplianceRuleEntry(
      123, directory_entry, ComplianceRuleEntry::State::ACTIVE,
      ComplianceRuleSchema())};
    fixture.on_request<LoadDirectoryEntryComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry) {
        REQUIRE(received_entry == directory_entry);
        request.SetResult(expected_entries);
      });
    auto received_entries =
      REQUIRE_NO_THROW(fixture.m_client->load(directory_entry));
    REQUIRE(received_entries == expected_entries);
  }

  TEST_CASE("add_compliance_rule_entry") {
    auto fixture = Fixture();
    auto directory_entry = DirectoryEntry::MakeAccount(2, "add_account");
    auto schema = ComplianceRuleSchema("test_rule", {});
    auto state = ComplianceRuleEntry::State::ACTIVE;
    auto expected_id = ComplianceRuleEntry::Id(42);
    fixture.on_request<AddComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry, auto received_state,
          const auto& received_schema) {
        REQUIRE(received_entry == directory_entry);
        REQUIRE(received_state == state);
        REQUIRE(received_schema == schema);
        request.SetResult(expected_id);
      });
    auto id =
      REQUIRE_NO_THROW(fixture.m_client->add(directory_entry, state, schema));
    REQUIRE(id == expected_id);
  }

  TEST_CASE("update_compliance_rule_entry") {
    auto fixture = Fixture();
    auto directory_entry = DirectoryEntry::MakeAccount(3, "update_account");
    auto entry = ComplianceRuleEntry(100, directory_entry,
      ComplianceRuleEntry::State::ACTIVE,
      ComplianceRuleSchema("update_rule", {}));
    auto is_called = Async<bool>();
    fixture.on_request<UpdateComplianceRuleEntryService>(
      [&] (auto& request, const auto& received_entry) {
        REQUIRE(received_entry == entry);
        request.SetResult();
        is_called.GetEval().SetResult(true);
      });
    REQUIRE_NO_THROW(fixture.m_client->update(entry));
    REQUIRE(is_called.Get());
  }

  TEST_CASE("remove_compliance_rule_entry") {
    auto fixture = Fixture();
    auto expected_id = ComplianceRuleEntry::Id(77);
    auto is_called = Async<bool>();
    fixture.on_request<DeleteComplianceRuleEntryService>(
      [&] (auto& request, auto received_id) {
        REQUIRE(received_id == expected_id);
        request.SetResult();
        is_called.GetEval().SetResult(true);
      });
    REQUIRE_NO_THROW(fixture.m_client->remove(expected_id));
    REQUIRE(is_called.Get());
  }

  TEST_CASE("report_compliance_rule_violation") {
    auto fixture = Fixture();
    auto record = ComplianceRuleViolationRecord();
    record.m_account = DirectoryEntry::MakeAccount(4, "report_account");
    record.m_order_id = 1234;
    record.m_rule_id = 5678;
    record.m_schema_name = "test_schema";
    record.m_reason = "violation reason";
    record.m_timestamp = time_from_string("2024-07-25 15:30:00");
    auto is_called = Async<bool>();
    fixture.on_message<ReportComplianceRuleViolationMessage>(
      [&] (auto& client, const auto& received_record) {
        REQUIRE(received_record == record);
        is_called.GetEval().SetResult(true);
      });
    REQUIRE_NOTHROW(fixture.m_client->report(record));
    REQUIRE(is_called.Get());
  }

  TEST_CASE("monitor_compliance_rule_entries") {
    auto fixture = Fixture();
    auto account = DirectoryEntry::MakeAccount(5, "monitor_account");
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
        server_side_client = &request.GetClient();
        request.SetResult(initial_entries);
      });
    auto queue = std::make_shared<Queue<ComplianceRuleEntry>>();
    auto snapshot = std::vector<ComplianceRuleEntry>();
    REQUIRE_NOTHROW(fixture.m_client->monitor_compliance_rule_entries(
      account, queue, Store(snapshot)));
    REQUIRE(snapshot == initial_entries);
    auto updated_entry = ComplianceRuleEntry(202, account,
      ComplianceRuleEntry::State::ACTIVE, ComplianceRuleSchema("ruleC", {}));
    SendRecordMessage<ComplianceRuleEntryMessage>(
      *server_side_client, updated_entry);
    REQUIRE(queue->Pop() == updated_entry);
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
        server_side_client = &request.GetClient();
        request.SetResult(reconnected_entries);
        reconnect_async.GetEval().SetResult();
      });
    server_side_client->Close();
    reconnect_async.Get();
    REQUIRE(queue->Pop() == reconnected_entries[0]);
    REQUIRE(queue->Pop() == reconnected_entries[2]);
  }
}
