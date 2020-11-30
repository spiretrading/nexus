#ifndef NEXUS_CHARTING_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_CHARTING_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/ChartingService/VirtualChartingClient.hpp"
#include "Nexus/ChartingServiceTests/ChartingServiceTests.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"

namespace Nexus::ChartingService::Tests {

  /**
   * Wraps most components needed to run an instance of the ChartingService with
   * helper functions.
   */
  class ChartingServiceTestEnvironment {
    public:

      /**
       * Constructs a ChartingServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param marketDataClient The MarketDataClient to use.
       */
      ChartingServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        std::shared_ptr<MarketDataService::VirtualMarketDataClient>
        marketDataClient);

      ~ChartingServiceTestEnvironment();

      /**
       * Builds a new ChartingClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the ChartingClient.
       */
      std::unique_ptr<VirtualChartingClient> MakeClient(
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
            MetaChartingServlet<std::shared_ptr<
              MarketDataService::VirtualMarketDataClient>>,
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
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;

      ChartingServiceTestEnvironment(
        const ChartingServiceTestEnvironment&) = delete;
      ChartingServiceTestEnvironment& operator =(
        const ChartingServiceTestEnvironment&) = delete;
  };

  inline ChartingServiceTestEnvironment::ChartingServiceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
    std::shared_ptr<MarketDataService::VirtualMarketDataClient>
    marketDataClient)
    : m_container(Beam::Initialize(std::move(serviceLocatorClient),
        Beam::Initialize(std::move(marketDataClient))), &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline ChartingServiceTestEnvironment::~ChartingServiceTestEnvironment() {
    Close();
  }

  inline std::unique_ptr<VirtualChartingClient>
      ChartingServiceTestEnvironment::MakeClient(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return MakeVirtualChartingClient(
      std::make_unique<ChartingClient<ServiceProtocolClientBuilder>>(
        ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
          std::bind(boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
            "test_charting_client", std::ref(m_serverConnection)),
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>())));
  }

  inline void ChartingServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
