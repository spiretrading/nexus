#include <boost/functional/factory.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
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
using namespace Nexus::MarketDataService;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaAdministrationServlet<ServiceLocatorClientBox,
        LocalAdministrationDataStore*, FixedTimeClient*>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    optional<ServiceLocatorClientBox> m_servlet_service_locator_client;
    LocalAdministrationDataStore m_data_store;
    std::shared_ptr<TestServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    DirectoryEntry m_admin_account;
    std::unique_ptr<TestServiceProtocolClient> m_admin_client;

    Fixture()
      : m_time_client(time_from_string("2024-07-04 12:00:00")),
        m_server_connection(std::make_shared<TestServerConnection>()) {
      auto servlet_account = make_account(
        "administration_service", DirectoryEntry::GetStarDirectory());
      m_servlet_service_locator_client =
        m_service_locator_environment.MakeClient(servlet_account.m_name, "");
      m_container.emplace(Initialize(*m_servlet_service_locator_client,
        Initialize(&m_service_locator_environment.GetRoot(),
          EntitlementDatabase(), &m_data_store, &m_time_client)),
          m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      auto admin_group =
        m_service_locator_environment.GetRoot().LoadDirectoryEntry(
          DirectoryEntry::GetStarDirectory(), "administrators");
      make_account("admin", admin_group);
      std::tie(m_admin_account, m_admin_client) = make_client("admin");
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
      RegisterAdministrationServices(Store(protocol_client->GetSlots()));
      RegisterAdministrationMessages(Store(protocol_client->GetSlots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.GetAccount(), std::move(protocol_client));
    }
  };
}

TEST_SUITE("AdministrationServlet") {
  TEST_CASE("directories_initialized") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    REQUIRE_NOTHROW(service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "administrators"));
    REQUIRE_NOTHROW(service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services"));
    REQUIRE_NOTHROW(service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "trading_groups"));
  }

  TEST_CASE("load_accounts_by_roles") {
    auto fixture = Fixture();
    auto& service_locator = fixture.m_service_locator_environment.GetRoot();
    auto admin_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "administrators");
    auto services_group = service_locator.LoadDirectoryEntry(
      DirectoryEntry::GetStarDirectory(), "services");
    auto admin_account = fixture.make_account("b", admin_group);
    auto service_account = fixture.make_account("c", services_group);
    auto roles = AccountRoles();
    roles.Set(AccountRole::ADMINISTRATOR);
    auto result =
      fixture.m_admin_client->SendRequest<LoadAccountsByRolesService>(roles);
    REQUIRE(result.size() == 2);
    REQUIRE(std::find(result.begin(), result.end(), fixture.m_admin_account) !=
      result.end());
    REQUIRE(
      std::find(result.begin(), result.end(), admin_account) != result.end());
    roles.Set(AccountRole::SERVICE);
    result =
      fixture.m_admin_client->SendRequest<LoadAccountsByRolesService>(roles);
    REQUIRE(result.size() == 3);
    REQUIRE(std::find(result.begin(), result.end(), fixture.m_admin_account) !=
      result.end());
    REQUIRE(
      std::find(result.begin(), result.end(), admin_account) != result.end());
    REQUIRE(
      std::find(result.begin(), result.end(), service_account) != result.end());
    roles.Unset(AccountRole::ADMINISTRATOR);
    result =
      fixture.m_admin_client->SendRequest<LoadAccountsByRolesService>(roles);
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == service_account);
  }

  TEST_CASE("load_account_identity") {
    auto fixture = Fixture();
    auto account =
      fixture.make_account("test_account", DirectoryEntry::GetStarDirectory());
    auto identity = AccountIdentity();
    identity.m_first_name = "Riley";
    identity.m_last_name = "Miller";
    fixture.m_data_store.store(account, identity);
    auto result = fixture.m_admin_client->SendRequest<
      LoadAccountIdentityService>(account);
    REQUIRE(result.m_first_name == identity.m_first_name);
    REQUIRE(result.m_last_name == identity.m_last_name);
  }
}
