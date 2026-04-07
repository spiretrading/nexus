#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/ServicesTests/TestServices.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDriver.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultDestinations;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  struct Fixture {
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaOrderExecutionServlet<FixedTimeClient*, ServiceLocatorClient,
        UidClient, AdministrationClient, TestOrderExecutionDriver*,
        LocalOrderExecutionDataStore*>>;
    FixedTimeClient m_time_client;
    ServiceLocatorTestEnvironment m_service_locator_environment;
    UidServiceTestEnvironment m_uid_environment;
    AdministrationServiceTestEnvironment m_administration_environment;
    std::shared_ptr<Queue<std::shared_ptr<TestOrderExecutionDriver::Operation>>>
      m_driver_operations;
    TestOrderExecutionDriver m_driver;
    optional<ServiceLocatorClient> m_servlet_service_locator_client;
    optional<UidClient> m_uid_client;
    optional<AdministrationClient> m_servlet_administration_client;
    LocalOrderExecutionDataStore m_data_store;
    std::shared_ptr<LocalServerConnection> m_server_connection;
    optional<ServletContainer> m_container;

    Fixture()
        : m_time_client(time_from_string("2025-04-12 13:33:12:55")),
          m_administration_environment(
            make_administration_service_test_environment(
              m_service_locator_environment)),
          m_driver_operations(std::make_shared<
            Queue<std::shared_ptr<TestOrderExecutionDriver::Operation>>>()),
          m_driver(m_driver_operations),
          m_server_connection(std::make_shared<LocalServerConnection>()) {
      auto servlet_account =
        m_service_locator_environment.get_root().make_account(
          "order_execution_service", "", DirectoryEntry::STAR_DIRECTORY);
      m_administration_environment.make_administrator(servlet_account);
      m_service_locator_environment.get_root().store(
        servlet_account, DirectoryEntry::STAR_DIRECTORY, Permissions(~0));
      m_servlet_service_locator_client.emplace(
        m_service_locator_environment.make_client(servlet_account.m_name, ""));
      m_servlet_administration_client.emplace(
        m_administration_environment.make_client(
          Ref(*m_servlet_service_locator_client)));
      m_container.emplace(init(
        *m_servlet_service_locator_client, init(pos_infin, DEFAULT_VENUES,
          DEFAULT_DESTINATIONS, &m_time_client,
          *m_servlet_service_locator_client, m_uid_environment.make_client(),
          *m_servlet_administration_client, &m_driver, &m_data_store)),
        m_server_connection, factory<std::unique_ptr<TriggerTimer>>());
    }
  };
}

TEST_SUITE("OrderExecutionServlet") {
  TEST_CASE("foo") {
    auto fixture = Fixture();
  }
}
