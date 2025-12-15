#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaComplianceServlet<ServiceLocatorClient, AdministrationClient,
        LocalComplianceRuleDataStore*, FixedTimeClient*>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<AdministrationClient> m_servlet_administration_client;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    LocalComplianceRuleDataStore m_data_store;
    optional<ServletContainer> m_container;
    DirectoryEntry m_client_account;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.get_root().make_account(
        name, "", parent);
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.make_client(name, "");
      auto authenticator = SessionAuthenticator(Ref(service_locator_client));
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        std::make_unique<LocalClientChannel>(name, *m_server_connection),
        init());
      register_compliance_services(out(protocol_client->get_slots()));
      register_compliance_messages(out(protocol_client->get_slots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.get_account(), std::move(protocol_client));
    }

    Fixture()
      : m_time_client(time_from_string("2025-07-03 13:00:00")),
        m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)),
        m_server_connection(std::make_shared<LocalServerConnection>()) {
      auto servlet_account =
        make_account("compliance_service", DirectoryEntry::STAR_DIRECTORY);
      m_service_locator_environment.get_root().store(
        servlet_account, DirectoryEntry::STAR_DIRECTORY, Permissions(~0));
      m_servlet_service_locator_client.emplace(
        m_service_locator_environment.make_client(servlet_account.m_name, ""));
      m_servlet_administration_client.emplace(
        m_administration_environment.make_client(
          Ref(*m_servlet_service_locator_client)));
      m_container.emplace(init(*m_servlet_service_locator_client,
        init(&m_service_locator_environment.get_root(),
          *m_servlet_administration_client, &m_data_store, &m_time_client)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_account =
        make_account("client", DirectoryEntry::STAR_DIRECTORY);
      std::tie(m_client_account, m_client) = make_client("client");
    }
  };
}

TEST_SUITE("ComplianceServlet") {
  TEST_CASE("load_directory_entry_compliance_rule_entry") {
    auto fixture = Fixture();
    auto second_account =
      fixture.make_account("second", DirectoryEntry::STAR_DIRECTORY);
    auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
    auto schema = ComplianceRuleSchema("TestRule", {});
    auto entry = ComplianceRuleEntry(
      id, second_account, ComplianceRuleEntry::State::ACTIVE, schema);
    fixture.m_data_store.store(entry);

    SUBCASE("without_permission") {
      REQUIRE_THROWS_AS(fixture.m_client->template send_request<
        LoadDirectoryEntryComplianceRuleEntryService>(second_account),
        ServiceRequestException);
    }

    SUBCASE("with_permission") {
      fixture.m_service_locator_environment.get_root().store(
        fixture.m_client_account, second_account, Permission::READ);
      auto result = fixture.m_client->template send_request<
        LoadDirectoryEntryComplianceRuleEntryService>(second_account);
      REQUIRE(result.size() == 1);
      REQUIRE(result[0] == entry);
    }
  }

  TEST_CASE("add_compliance_rule_entry") {
    auto fixture = Fixture();
    auto schema = ComplianceRuleSchema("TestRule", {});

    SUBCASE("without_permission") {
      REQUIRE_THROWS_AS(
        fixture.m_client->template send_request<AddComplianceRuleEntryService>(
          fixture.m_client_account, ComplianceRuleEntry::State::ACTIVE, schema),
          ServiceRequestException);
    }

    SUBCASE("with_permission") {
      fixture.m_administration_environment.make_administrator(
        fixture.m_client_account);
      auto id = fixture.m_client->template send_request<
        AddComplianceRuleEntryService>(fixture.m_client_account,
          ComplianceRuleEntry::State::ACTIVE, schema);
      auto rules = fixture.m_data_store.load_compliance_rule_entries(
        fixture.m_client_account);
      REQUIRE(rules.size() == 1);
      REQUIRE(rules[0].get_id() == id);
      REQUIRE(rules[0].get_directory_entry() == fixture.m_client_account);
      REQUIRE(rules[0].get_schema() == schema);
    }
  }

  TEST_CASE("update_compliance_rule_entry") {
    auto fixture = Fixture();
    auto second_account =
      fixture.make_account("second", DirectoryEntry::STAR_DIRECTORY);
    auto schema = ComplianceRuleSchema("TestRule", {});
    auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
    auto entry = ComplianceRuleEntry(
      id, second_account, ComplianceRuleEntry::State::ACTIVE, schema);
    fixture.m_data_store.store(entry);

    SUBCASE("without_permission") {
      REQUIRE_THROWS_AS(fixture.m_client->template send_request<
        UpdateComplianceRuleEntryService>(entry), ServiceRequestException);
    }

    SUBCASE("with_permission") {
      auto updated_entry = entry;
      updated_entry.set_state(ComplianceRuleEntry::State::PASSIVE);
      fixture.m_administration_environment.make_administrator(
        fixture.m_client_account);
      fixture.m_client->template send_request<UpdateComplianceRuleEntryService>(
        updated_entry);
      auto updated_rules =
        fixture.m_data_store.load_compliance_rule_entries(second_account);
      REQUIRE(updated_rules.size() == 1);
      REQUIRE(updated_rules[0] == updated_entry);
    }
  }

  TEST_CASE("delete_compliance_rule_entry") {
    auto fixture = Fixture();
    auto schema = ComplianceRuleSchema("TestRule", {});
    auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
    auto entry = ComplianceRuleEntry(
      id, fixture.m_client_account, ComplianceRuleEntry::State::ACTIVE, schema);
    fixture.m_data_store.store(entry);

    SUBCASE("without_permission") {
      REQUIRE_THROWS_AS(
        fixture.m_client->template send_request<
          DeleteComplianceRuleEntryService>(id), ServiceRequestException);
    }

    SUBCASE("with_permission") {
      fixture.m_administration_environment.make_administrator(
        fixture.m_client_account);
      fixture.m_client->template send_request<DeleteComplianceRuleEntryService>(
        id);
      auto rules = fixture.m_data_store.load_compliance_rule_entries(
        fixture.m_client_account);
      REQUIRE(rules.empty());
      auto deleted_entry = fixture.m_data_store.load_compliance_rule_entry(id);
      REQUIRE(!deleted_entry);
    }
  }

  TEST_CASE("report_compliance_rule_violation") {
    auto fixture = Fixture();
    auto schema = ComplianceRuleSchema("TestRule", {});
    auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
    auto entry = ComplianceRuleEntry(
      id, fixture.m_client_account, ComplianceRuleEntry::State::ACTIVE, schema);
    fixture.m_data_store.store(entry);
    auto violation = ComplianceRuleViolationRecord();
    violation.m_account = fixture.m_client_account;
    violation.m_order_id = 123;
    violation.m_rule_id = id;
    violation.m_schema_name = schema.get_name();
    violation.m_reason = "Broke the rules.";

    SUBCASE("without_permission") {
      send_record_message<ReportComplianceRuleViolationMessage>(
        *fixture.m_client,  violation);
    }

    SUBCASE("with_permission") {
      fixture.m_administration_environment.make_administrator(
        fixture.m_client_account);
      send_record_message<ReportComplianceRuleViolationMessage>(
        *fixture.m_client,  violation);
    }
  }

  TEST_CASE("monitor_compliance_rule_entry") {
    auto fixture = Fixture();
    auto second_account =
      fixture.make_account("second", DirectoryEntry::STAR_DIRECTORY);
    auto third_account =
      fixture.make_account("third", DirectoryEntry::STAR_DIRECTORY);
    auto initial_second_account_entry = [&] {
      auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
      auto entry = ComplianceRuleEntry(id, second_account,
        ComplianceRuleEntry::State::PASSIVE,
        ComplianceRuleSchema("Second Account Rule", {}));
      fixture.m_data_store.store(entry);
      return entry;
    }();
    auto initial_third_account_entry = [&] {
      auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
      auto entry = ComplianceRuleEntry(id, third_account,
        ComplianceRuleEntry::State::ACTIVE,
        ComplianceRuleSchema("Third Account Rule", {}));
      fixture.m_data_store.store(entry);
      return entry;
    }();
    auto schema = ComplianceRuleSchema("TestRule", {});

    SUBCASE("without_permission") {
      REQUIRE_THROWS_AS(fixture.m_client->template send_request<
        MonitorComplianceRuleEntryService>(second_account),
        ServiceRequestException);
    }

    SUBCASE("with_permission") {
      fixture.m_service_locator_environment.get_root().store(
        fixture.m_client_account, second_account, Permission::READ);
      fixture.m_administration_environment.make_administrator(
        fixture.m_client_account);
      auto rules = fixture.m_client->template send_request<
        MonitorComplianceRuleEntryService>(second_account);
      REQUIRE(rules.size() == 1);
      REQUIRE(rules[0] == initial_second_account_entry);
      auto id = fixture.m_client->template send_request<
        AddComplianceRuleEntryService>(second_account,
          ComplianceRuleEntry::State::ACTIVE, schema);
      auto message = fixture.m_client->read_message();
      auto received_message = std::dynamic_pointer_cast<
        RecordMessage<ComplianceRuleEntryMessage, TestServiceProtocolClient>>(
          message);
      REQUIRE(received_message);
      REQUIRE(
        received_message->get_record().compliance_rule_entry.get_id() == id);
      REQUIRE(received_message->get_record().
        compliance_rule_entry.get_directory_entry() == second_account);
      REQUIRE(
        received_message->get_record().compliance_rule_entry.get_schema() ==
          schema);
      auto updated_entry = received_message->get_record().compliance_rule_entry;
      updated_entry.set_state(ComplianceRuleEntry::State::PASSIVE);
      fixture.m_client->template send_request<
        UpdateComplianceRuleEntryService>(updated_entry);
      message = fixture.m_client->read_message();
      received_message = std::dynamic_pointer_cast<
        RecordMessage<ComplianceRuleEntryMessage, TestServiceProtocolClient>>(
          message);
      REQUIRE(received_message);
      REQUIRE(
        received_message->get_record().compliance_rule_entry.get_id() == id);
      REQUIRE(
        received_message->get_record().compliance_rule_entry.get_state() ==
          ComplianceRuleEntry::State::PASSIVE);
      fixture.m_client->template send_request<
        DeleteComplianceRuleEntryService>(initial_third_account_entry.get_id());
      fixture.m_client->template send_request<
        DeleteComplianceRuleEntryService>(id);
      message = fixture.m_client->read_message();
      received_message = std::dynamic_pointer_cast<
        RecordMessage<ComplianceRuleEntryMessage, TestServiceProtocolClient>>(
          message);
      REQUIRE(received_message);
      REQUIRE(
        received_message->get_record().compliance_rule_entry.get_id() == id);
      REQUIRE(
        received_message->get_record().compliance_rule_entry.get_state() ==
          ComplianceRuleEntry::State::DELETED);
    }
  }
}
