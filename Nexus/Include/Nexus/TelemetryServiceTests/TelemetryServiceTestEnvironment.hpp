#ifndef NEXUS_TELEMETRY_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_TELEMETRY_SERVICE_TEST_ENVIRONMENT_HPP
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
#include <Beam/TimeService/TimeClientBox.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"
#include "Nexus/TelemetryService/TelemetryClient.hpp"
#include "Nexus/TelemetryService/TelemetryClientBox.hpp"
#include "Nexus/TelemetryService/TelemetryServlet.hpp"
#include "Nexus/TelemetryServiceTests/TelemetryServiceTests.hpp"

namespace Nexus::TelemetryService::Tests {

  /**
   * Wraps most components needed to run an instance of the TelemetryService
   * with helper functions.
   */
  class TelemetryServiceTestEnvironment {
    public:

      /**
       * Constructs a TelemetryServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param timeClient The TimeClient to use.
       * @param administrationClient The AdministrationClient to use.
       */
      TelemetryServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        Beam::TimeService::TimeClientBox timeClient,
        AdministrationService::AdministrationClientBox administrationClient);

      ~TelemetryServiceTestEnvironment();

      /**
       * Returns a new TelemetryClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the TelemetryClient.
       */
      TelemetryClientBox MakeClient(
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
            MetaTelemetryServlet<Beam::TimeService::TimeClientBox,
              AdministrationService::AdministrationClientBox,
              LocalTelemetryDataStore*>,
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
            Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      Beam::TimeService::TimeClientBox m_timeClient;
      LocalTelemetryDataStore m_dataStore;
      ServerConnection m_serverConnection;
      boost::optional<ServiceProtocolServletContainer> m_container;

      TelemetryServiceTestEnvironment(const TelemetryServiceTestEnvironment&) =
        delete;
      TelemetryServiceTestEnvironment& operator =(
        const TelemetryServiceTestEnvironment&) = delete;
  };

  inline TelemetryServiceTestEnvironment::TelemetryServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
      Beam::TimeService::TimeClientBox timeClient,
      AdministrationService::AdministrationClientBox administrationClient)
      : m_timeClient(std::move(timeClient)) {
    m_container.emplace(Beam::Initialize(serviceLocatorClient,
      Beam::Initialize(m_timeClient, std::move(administrationClient),
        &m_dataStore)), &m_serverConnection,
      boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
  }

  inline TelemetryServiceTestEnvironment::~TelemetryServiceTestEnvironment() {
    Close();
  }

  inline TelemetryClientBox TelemetryServiceTestEnvironment::MakeClient(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return TelemetryClientBox(std::in_place_type<TelemetryClient<
        ServiceProtocolClientBuilder, Beam::TimeService::TimeClientBox>>,
      ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_telemetry_client", std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()),
      m_timeClient);
  }

  inline void TelemetryServiceTestEnvironment::Close() {
    m_container->Close();
  }
}

#endif
