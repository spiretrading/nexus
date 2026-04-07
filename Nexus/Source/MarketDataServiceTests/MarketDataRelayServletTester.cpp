#include <future>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataRelayServlet.hpp"
#include "Nexus/MarketDataServiceTests/TestMarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    using ServletContainer =
      TestAuthenticatedServiceProtocolServletContainer<
        MetaMarketDataRelayServlet<MarketDataClient, AdministrationClient>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    optional<AdministrationClient> m_servlet_administration_client;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;
    std::shared_ptr<Queue<std::shared_ptr<TestMarketDataClient::Operation>>>
      m_operations;
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
      Nexus::register_query_types(
        Beam::out(protocol_client->get_slots().get_registry()));
      register_market_data_registry_services(out(protocol_client->get_slots()));
      register_market_data_registry_messages(out(protocol_client->get_slots()));
      authenticator(*protocol_client);
      return std::tuple(
        service_locator_client.get_account(), std::move(protocol_client));
    }

    auto make_relay_client() {
      return std::make_unique<MarketDataClient>(
        std::in_place_type<TestMarketDataClient>, m_operations);
    }

    Fixture()
        : m_time_client(time_from_string("2024-07-04 12:00:00")),
          m_server_connection(std::make_shared<LocalServerConnection>()),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_operations(std::make_shared<Queue<
            std::shared_ptr<TestMarketDataClient::Operation>>>()) {
      auto servlet_account =
        make_account("market_data_service", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator_environment.get_root().store(
        servlet_account, DirectoryEntry::STAR_DIRECTORY, Permissions(~0));
      m_servlet_service_locator_client.emplace(
        m_service_locator_environment.make_client(servlet_account.m_name, ""));
      m_servlet_administration_client.emplace(
        m_administration_environment.make_client(
          Ref(*m_servlet_service_locator_client)));
      m_container.emplace(
        init(*m_servlet_service_locator_client, init(seconds(100),
          std::bind_front(&Fixture::make_relay_client, this), 1, 1,
          m_administration_environment.make_client(
            Ref(*m_servlet_service_locator_client)))),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
      m_client_account =
        make_account("client", DirectoryEntry::STAR_DIRECTORY);
      auto global_entitlement = m_servlet_administration_client->
        load_entitlements().get_entries().front().m_group_entry;
      m_servlet_administration_client->store_entitlements(
        m_client_account, {global_entitlement});
      std::tie(m_client_account, m_client) = make_client("client");
    }
  };
}

TEST_SUITE("MarketDataRegistryServlet") {
  TEST_CASE("query_security_info") {
    auto fixture = Fixture();
    auto security = Security("TST", TSX);
    auto query = SecurityInfoQuery();
    query.set_index(security);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    auto query_thread = std::async(std::launch::async, [&] {
      return fixture.m_client->send_request<QuerySecurityInfoService>(query);
    });
    auto operation = fixture.m_operations->pop();
    auto& security_info_operation =
      std::get<TestMarketDataClient::SecurityInfoQueryOperation>(*operation);
    REQUIRE(security_info_operation.m_query.get_index() == security);
    auto security_info = SecurityInfo();
    security_info.m_security = security;
    security_info.m_name = "Test";
    security_info.m_sector = "Tech";
    security_info.m_board_lot = 100;
    security_info_operation.m_result.set({security_info});
    auto result = query_thread.get();
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == security_info);
  }
}
