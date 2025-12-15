#ifndef NEXUS_CHARTING_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_CHARTING_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/ChartingService/ServiceChartingClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::Tests {

  /**
   * Wraps most components needed to run an instance of the ChartingService with
   * helper functions.
   */
  class ChartingServiceTestEnvironment {
    public:

      /**
       * Constructs a ChartingServiceTestEnvironment.
       * @param service_locator_client The ServiceLocatorClient to use.
       * @param market_data_client The MarketDataClient to use.
       */
      ChartingServiceTestEnvironment(
        Beam::ServiceLocatorClient service_locator_client,
        MarketDataClient market_data_client);

      ~ChartingServiceTestEnvironment();

      /**
       * Returns a new ChartingClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the ChartingClient.
       */
      ChartingClient make_client(
        Beam::Ref<Beam::ServiceLocatorClient> service_locator_client);

      void close();

    private:
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          Beam::MetaAuthenticationServletAdapter<
            MetaChartingServlet<MarketDataClient>, Beam::ServiceLocatorClient>,
          Beam::LocalServerConnection*, Beam::BinarySender<Beam::SharedBuffer>,
          Beam::NullEncoder, std::shared_ptr<Beam::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocatorClient,
          Beam::MessageProtocol<std::unique_ptr<Beam::LocalClientChannel>,
            Beam::BinarySender<Beam::SharedBuffer>, Beam::NullEncoder>,
          Beam::TriggerTimer>;
      Beam::LocalServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;

      ChartingServiceTestEnvironment(
        const ChartingServiceTestEnvironment&) = delete;
      ChartingServiceTestEnvironment& operator =(
        const ChartingServiceTestEnvironment&) = delete;
  };

  inline ChartingServiceTestEnvironment::ChartingServiceTestEnvironment(
    Beam::ServiceLocatorClient service_locator_client,
    MarketDataClient market_data_client)
    : m_container(Beam::init(std::move(service_locator_client),
        Beam::init(std::move(market_data_client))), &m_server_connection,
        boost::factory<std::shared_ptr<Beam::TriggerTimer>>()) {}

  inline ChartingServiceTestEnvironment::~ChartingServiceTestEnvironment() {
    close();
  }

  inline ChartingClient ChartingServiceTestEnvironment::make_client(
      Beam::Ref<Beam::ServiceLocatorClient> service_locator_client) {
    return ChartingClient(std::in_place_type<
      ServiceChartingClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(Beam::Ref(service_locator_client),
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_charting_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void ChartingServiceTestEnvironment::close() {
    m_container.close();
  }
}

#endif
