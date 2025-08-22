#ifndef NEXUS_ORDER_EXECUTION_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_ORDER_EXECUTION_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/ServiceOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"

namespace Nexus::OrderExecutionService::Tests {

  /**
   * Wraps most components needed to run an instance of the
   * OrderExecutionService with helper functions.
   */
  class OrderExecutionServiceTestEnvironment {
    public:

      /**
       * Constructs an OrderExecutionServiceTestEnvironment using default venues
       * and destinations, and a MockOrderExecutionDriver.
       * @param service_locator_client The ServiceLocatorClient used by the
       *        servlet.
       * @param uid_client The UidClient used by the servlet.
       * @param administration_client The AdministrationClient used by the
       *        servlet.
       */
      OrderExecutionServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
        Beam::UidService::UidClientBox uid_client,
        AdministrationService::AdministrationClient administration_client);

      /**
       * Constructs an OrderExecutionServiceTestEnvironment using a
       * MockOrderExecutionDriver.
       * @param venues The available venues to trade.
       * @param destinations The available destinations to submit orders to.
       * @param service_locator_client The ServiceLocatorClient used by the
       *        servlet.
       * @param uid_client The UidClient used by the servlet.
       * @param administration_client The AdministrationClient used by the
       *        servlet.
       */
      OrderExecutionServiceTestEnvironment(VenueDatabase venues,
        DestinationDatabase destinations,
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
        Beam::UidService::UidClientBox uid_client,
        AdministrationService::AdministrationClient administration_client);

      /**
       * Constructs an OrderExecutionServiceTestEnvironment.
       * @param venues The available venues to trade.
       * @param destinations The available destinations to submit orders to.
       * @param service_locator_client The ServiceLocatorClient used by the
       *        servlet.
       * @param uid_client The UidClient used by the servlet.
       * @param administration_client The AdministrationClient used by the
       *        servlet.
       * @param time_client The TimeClient to use.
       * @param driver The OrderExecutionDriver to use.
       */
      OrderExecutionServiceTestEnvironment(VenueDatabase venues,
        DestinationDatabase destinations,
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
        Beam::UidService::UidClientBox uid_client,
        AdministrationService::AdministrationClient administration_client,
        Beam::TimeService::TimeClientBox time_client,
        OrderExecutionDriver driver);

      ~OrderExecutionServiceTestEnvironment();

      /** Returns the data store. */
      const LocalOrderExecutionDataStore& get_data_store() const;

      /** Returns the data store. */
      LocalOrderExecutionDataStore& get_data_store();

      /** Returns the driver used to manage submitted Orders. */
      OrderExecutionDriver& get_driver();

      /**
       * Returns a new OrderExecutionClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the OrderExecutionClient.
       */
      OrderExecutionClient make_client(
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client);

      void close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
          Beam::ServiceLocator::MetaAuthenticationServletAdapter<
            MetaOrderExecutionServlet<Beam::TimeService::TimeClientBox,
              Beam::ServiceLocator::ServiceLocatorClientBox,
              Beam::UidService::UidClientBox,
              AdministrationService::AdministrationClient,
              OrderExecutionDriver*, LocalOrderExecutionDataStore*>,
            Beam::ServiceLocator::ServiceLocatorClientBox>,
          ServerConnection*,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocator::ServiceLocatorClientBox,
          Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
            Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
            Beam::Codecs::NullEncoder>,
          Beam::Threading::TriggerTimer>;
      LocalOrderExecutionDataStore m_data_store;
      OrderExecutionDriver m_driver;
      ServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;

      OrderExecutionServiceTestEnvironment(
        const OrderExecutionServiceTestEnvironment&) = delete;
      OrderExecutionServiceTestEnvironment& operator =(
        const OrderExecutionServiceTestEnvironment&) = delete;
  };

  /**
   * Constructs an OrderExecutionServiceTestEnvironment using defaults settings
   * from a ServiceLocatorTestEnvironment.
   * @param service_locator_environment The ServiceLocatorTestEnvironment used
   *        to create default settings.
   * @param uid_environment The UidTestEnvironment used for order ids.
   * @param administration_environment The AdministrationTestEnvironment used
   *        to verify permissions.
   */
  inline OrderExecutionServiceTestEnvironment
      make_order_execution_service_test_environment(
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
          service_locator_environment,
        Beam::UidService::Tests::UidServiceTestEnvironment& uid_environment,
        AdministrationService::Tests::AdministrationServiceTestEnvironment&
          administration_service_test_environment) {
    auto account = service_locator_environment.GetRoot().MakeAccount(
      "order_execution_service", "1234",
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    service_locator_environment.GetRoot().StorePermissions(account,
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      Beam::ServiceLocator::Permissions().Set(
        Beam::ServiceLocator::Permission::READ).Set(
        Beam::ServiceLocator::Permission::MOVE).Set(
        Beam::ServiceLocator::Permission::ADMINISTRATE));
    administration_service_test_environment.make_administrator(account);
    auto service_locator_client =
      service_locator_environment.MakeClient("order_execution_service", "1234");
    auto administration_client =
      administration_service_test_environment.make_client(
        service_locator_client);
    return OrderExecutionServiceTestEnvironment(service_locator_client,
      uid_environment.MakeClient(), administration_client);
  }

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
      Beam::UidService::UidClientBox uid_client,
      AdministrationService::AdministrationClient administration_client)
    : OrderExecutionServiceTestEnvironment(DEFAULT_VENUES, DEFAULT_DESTINATIONS,
        std::move(service_locator_client), std::move(uid_client),
        std::move(administration_client)) {}

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(VenueDatabase venues,
      DestinationDatabase destinations,
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
      Beam::UidService::UidClientBox uid_client,
      AdministrationService::AdministrationClient administration_client)
    : OrderExecutionServiceTestEnvironment(std::move(venues),
        std::move(destinations), std::move(service_locator_client),
        std::move(uid_client), std::move(administration_client),
        Beam::TimeService::TimeClientBox(
          std::in_place_type<Beam::TimeService::IncrementalTimeClient>),
        OrderExecutionDriver(std::in_place_type<MockOrderExecutionDriver>)) {}

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(VenueDatabase venues,
      DestinationDatabase destinations,
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
      Beam::UidService::UidClientBox uid_client,
      AdministrationService::AdministrationClient administration_client,
      Beam::TimeService::TimeClientBox time_client, OrderExecutionDriver driver)
    : m_driver(std::move(driver)),
      m_container(Beam::Initialize(service_locator_client, Beam::Initialize(
        boost::posix_time::pos_infin, std::move(venues),
        std::move(destinations), std::move(time_client),
        service_locator_client, std::move(uid_client),
        std::move(administration_client), &m_driver, &m_data_store)),
        &m_server_connection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline OrderExecutionServiceTestEnvironment::
      ~OrderExecutionServiceTestEnvironment() {
    close();
  }

  inline const LocalOrderExecutionDataStore&
      OrderExecutionServiceTestEnvironment::get_data_store() const {
    return m_data_store;
  }

  inline LocalOrderExecutionDataStore&
      OrderExecutionServiceTestEnvironment::get_data_store() {
    return m_data_store;
  }

  inline OrderExecutionDriver&
      OrderExecutionServiceTestEnvironment::get_driver() {
    return m_driver;
  }

  inline OrderExecutionClient OrderExecutionServiceTestEnvironment::make_client(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return OrderExecutionClient(std::in_place_type<
      ServiceOrderExecutionClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(serviceLocatorClient,
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_order_execution_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void OrderExecutionServiceTestEnvironment::close() {
    m_container.Close();
  }
}

#endif
