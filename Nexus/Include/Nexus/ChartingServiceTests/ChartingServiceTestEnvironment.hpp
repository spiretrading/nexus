#ifndef NEXUS_CHARTING_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_CHARTING_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/ChartingService/ServiceChartingClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::ChartingService::Tests {

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
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
        MarketDataService::MarketDataClient market_data_client);

      ~ChartingServiceTestEnvironment();

      /**
       * Returns a new ChartingClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the ChartingClient.
       */
      ChartingClient make_client(
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
            MetaChartingServlet<MarketDataService::MarketDataClient>,
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
      ServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;

      ChartingServiceTestEnvironment(
        const ChartingServiceTestEnvironment&) = delete;
      ChartingServiceTestEnvironment& operator =(
        const ChartingServiceTestEnvironment&) = delete;
  };

  inline ChartingServiceTestEnvironment::ChartingServiceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client,
    MarketDataService::MarketDataClient market_data_client)
    : m_container(Beam::Initialize(std::move(service_locator_client),
        Beam::Initialize(std::move(market_data_client))), &m_server_connection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline ChartingServiceTestEnvironment::~ChartingServiceTestEnvironment() {
    close();
  }

  inline ChartingClient ChartingServiceTestEnvironment::make_client(
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client) {
    return ChartingClient(std::in_place_type<
      ServiceChartingClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::move(service_locator_client),
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_charting_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void ChartingServiceTestEnvironment::close() {
    m_container.Close();
  }
}

#endif
