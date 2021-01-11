#ifndef NEXUS_DEFINITIONS_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_DEFINITIONS_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsClientBox.hpp"
#include "Nexus/DefinitionsService/DefinitionsServlet.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTests.hpp"

namespace Nexus::DefinitionsService::Tests {

  /** Provides DefinitionsService related classes for testing purposes. */
  class DefinitionsServiceTestEnvironment {
    public:

      /**
       * Constructs a DefinitionsServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       */
      DefinitionsServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      ~DefinitionsServiceTestEnvironment();

      /**
       * Returns a new DefinitionsClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the DefinitionsClient.
       */
      DefinitionsClientBox MakeClient(
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
            MetaDefinitionsServlet,
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

      DefinitionsServiceTestEnvironment(
        const DefinitionsServiceTestEnvironment&) = delete;
      DefinitionsServiceTestEnvironment& operator =(
        const DefinitionsServiceTestEnvironment&) = delete;
  };

  inline DefinitionsServiceTestEnvironment::DefinitionsServiceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient)
    : m_container(Beam::Initialize(std::move(serviceLocatorClient),
        Beam::Initialize("1", "Spire Trading Inc.", GetDefaultTimeZoneTable(),
          GetDefaultCountryDatabase(), GetDefaultCurrencyDatabase(),
          GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
          std::vector<ExchangeRate>(),
          std::vector<Compliance::ComplianceRuleSchema>())),
        &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline DefinitionsServiceTestEnvironment::
      ~DefinitionsServiceTestEnvironment() {
    Close();
  }

  inline DefinitionsClientBox DefinitionsServiceTestEnvironment::MakeClient(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return DefinitionsClientBox(std::in_place_type<DefinitionsClient<
      ServiceProtocolClientBuilder>>, ServiceProtocolClientBuilder(
        serviceLocatorClient,
        std::bind(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_definitions_client", std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void DefinitionsServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
