#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::Compliance;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaComplianceServlet<ServiceLocatorClientBox, AdministrationClient,
        LocalComplianceRuleDataStore*, FixedTimeClient*>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    optional<ServiceLocatorClientBox> m_servlet_service_locator_client;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<AdministrationClient> m_servlet_administration_client;
    std::shared_ptr<TestServerConnection> m_server_connection;
    LocalComplianceRuleDataStore m_data_store;
    optional<ServletContainer> m_container;
    DirectoryEntry m_client_account;
    std::unique_ptr<TestServiceProtocolClient> m_client;

    Fixture()
      : m_time_client(time_from_string("2025-07-03 13:00:00")),
        m_administration_environment(
          make_administration_service_test_environment(
            m_service_locator_environment)),
        m_server_connection(std::make_shared<TestServerConnection>()) {
      auto servlet_account =
        make_account("compliance_service", DirectoryEntry::GetStarDirectory());
      m_service_locator_environment.GetRoot().StorePermissions(
        servlet_account, DirectoryEntry::GetStarDirectory(), Permissions(~0));
      m_servlet_service_locator_client =
        m_service_locator_environment.MakeClient(servlet_account.m_name, "");
      m_servlet_administration_client =
        m_administration_environment.make_client(
          *m_servlet_service_locator_client);
      m_container.emplace(Initialize(*m_servlet_service_locator_client,
        Initialize(&m_service_locator_environment.GetRoot(),
          *m_servlet_administration_client, &m_data_store, &m_time_client)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_account =
        make_account("client", DirectoryEntry::GetStarDirectory());
      std::tie(m_client_account, m_client) = make_client("client");
    }

    auto make_account(const std::string& name, const DirectoryEntry& parent) {
      return m_service_locator_environment.GetRoot().MakeAccount(
        name, "", parent);
    }

    auto make_client(const std::string& name) {
      auto service_locator_client =
        m_service_locator_environment.MakeClient(name, "");
      auto authenticator = SessionAuthenticator(service_locator_client);
      auto protocol_client = std::make_unique<TestServiceProtocolClient>(
        Initialize(name, *m_server_connection), Initialize());
      RegisterComplianceServices(Store(protocol_client->GetSlots()));
      RegisterComplianceMessages(Store(protocol_client->GetSlots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.GetAccount(), std::move(protocol_client));
    }
  };
}

TEST_SUITE("ComplianceServlet") {
  TEST_CASE("load_directory_entry_compliance_rule_entry") {
    auto fixture = Fixture();
    auto second_account =
      fixture.make_account("second", DirectoryEntry::GetStarDirectory());
    auto id = fixture.m_data_store.load_next_compliance_rule_entry_id();
    auto schema = ComplianceRuleSchema("TestRule", {});
    auto entry = ComplianceRuleEntry(
      id, second_account, ComplianceRuleEntry::State::ACTIVE, schema);
    fixture.m_data_store.store(entry);
    SUBCASE("fails_without_permission") {
      REQUIRE_THROWS_AS(fixture.m_client->template SendRequest<
        LoadDirectoryEntryComplianceRuleEntryService>(second_account),
        Beam::Services::ServiceRequestException);
    }
    SUBCASE("succeeds_with_permission") {
      fixture.m_service_locator_environment.GetRoot().StorePermissions(
        fixture.m_client_account, second_account, Permission::READ);
      auto result = fixture.m_client->template SendRequest<
        LoadDirectoryEntryComplianceRuleEntryService>(second_account);
      REQUIRE(result.size() == 1);
      REQUIRE(result[0] == entry);
    }
  }

  TEST_CASE("add_compliance_rule_entry") {
    auto fixture = Fixture();
    auto schema = ComplianceRuleSchema("TestRule", {});
    SUBCASE("fails_without_administrator_permission") {
      REQUIRE_THROWS_AS(
        fixture.m_client->template SendRequest<AddComplianceRuleEntryService>(
          fixture.m_client_account, ComplianceRuleEntry::State::ACTIVE, schema),
          Beam::Services::ServiceRequestException);
    }
    SUBCASE("succeeds_with_administrator_permission") {
      fixture.m_administration_environment.make_administrator(
        fixture.m_client_account);
      auto id = fixture.m_client->template SendRequest<
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
}
