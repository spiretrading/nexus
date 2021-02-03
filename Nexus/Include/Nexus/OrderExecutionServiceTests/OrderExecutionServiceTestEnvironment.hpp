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
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClientBox.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus::OrderExecutionService::Tests {

  /**
   * Wraps most components needed to run an instance of the
   * OrderExecutionService with helper functions.
   */
  class OrderExecutionServiceTestEnvironment {
    public:

      /**
       * Constructs an OrderExecutionServiceTestEnvironment.
       * @param marketDatabase The MarketDatabase to use.
       * @param destinationDatabase The DestinationDatabase to use.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param uidClient The UidClient to use.
       * @param administrationClient The AdministrationClient to use.
       */
      OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
        DestinationDatabase destinationDatabase,
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        Beam::UidService::UidClientBox uidClient,
        AdministrationService::AdministrationClientBox administrationClient);

      /**
       * Constructs an OrderExecutionServiceTestEnvironment.
       * @param marketDatabase The MarketDatabase to use.
       * @param destinationDatabase The DestinationDatabase to use.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param uidClient The UidClient to use.
       * @param administrationClient The AdministrationClient to use.
       * @param timeClient The TimeClient to use.
       * @param driver The OrderExecutionDriver to use.
       */
      OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
        DestinationDatabase destinationDatabase,
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        Beam::UidService::UidClientBox uidClient,
        AdministrationService::AdministrationClientBox administrationClient,
        Beam::TimeService::TimeClientBox timeClient,
        std::unique_ptr<VirtualOrderExecutionDriver> driver);

      ~OrderExecutionServiceTestEnvironment();

      /** Returns the driver used to manage submitted Orders. */
      VirtualOrderExecutionDriver& GetDriver();

      /**
       * Returns a new OrderExecutionClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the OrderExecutionClient.
       */
      OrderExecutionClientBox MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      void Close();

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
              AdministrationService::AdministrationClientBox,
              VirtualOrderExecutionDriver*, LocalOrderExecutionDataStore*>,
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
      LocalOrderExecutionDataStore m_dataStore;
      std::unique_ptr<VirtualOrderExecutionDriver> m_driver;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;

      OrderExecutionServiceTestEnvironment(
        const OrderExecutionServiceTestEnvironment&) = delete;
      OrderExecutionServiceTestEnvironment& operator =(
        const OrderExecutionServiceTestEnvironment&) = delete;
  };

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
      DestinationDatabase destinationDatabase,
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
      Beam::UidService::UidClientBox uidClient,
      AdministrationService::AdministrationClientBox administrationClient)
    : OrderExecutionServiceTestEnvironment(std::move(marketDatabase),
        std::move(destinationDatabase), std::move(serviceLocatorClient),
        std::move(uidClient), std::move(administrationClient),
        Beam::TimeService::TimeClientBox(
          std::in_place_type<Beam::TimeService::IncrementalTimeClient>),
        MakeVirtualOrderExecutionDriver<MockOrderExecutionDriver>(
          Beam::Initialize())) {}

  inline OrderExecutionServiceTestEnvironment::
    OrderExecutionServiceTestEnvironment(MarketDatabase marketDatabase,
      DestinationDatabase destinationDatabase,
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
      Beam::UidService::UidClientBox uidClient,
      AdministrationService::AdministrationClientBox administrationClient,
      Beam::TimeService::TimeClientBox timeClient,
      std::unique_ptr<VirtualOrderExecutionDriver> driver)
    : m_driver(std::move(driver)),
      m_container(Beam::Initialize(serviceLocatorClient, Beam::Initialize(
        boost::posix_time::pos_infin, std::move(marketDatabase),
        std::move(destinationDatabase), std::move(timeClient),
        serviceLocatorClient, std::move(uidClient),
        std::move(administrationClient), &*m_driver, &m_dataStore)),
        &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline OrderExecutionServiceTestEnvironment::
      ~OrderExecutionServiceTestEnvironment() {
    Close();
  }

  inline VirtualOrderExecutionDriver& OrderExecutionServiceTestEnvironment::
      GetDriver() {
    return *m_driver;
  }

  inline OrderExecutionClientBox
      OrderExecutionServiceTestEnvironment::MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return OrderExecutionClientBox(
      std::in_place_type<OrderExecutionClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(serviceLocatorClient,
        std::bind(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_order_execution_client", std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void OrderExecutionServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
