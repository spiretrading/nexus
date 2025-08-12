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
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/DefinitionsServlet.hpp"
#include "Nexus/DefinitionsService/ServiceDefinitionsClient.hpp"

namespace Nexus::DefinitionsService::Tests {

  /** Provides DefinitionsService related classes for testing purposes. */
  class DefinitionsServiceTestEnvironment {
    public:

      /**
       * Constructs a DefinitionsServiceTestEnvironment.
       * @param service_locator_client The ServiceLocatorClient to use.
       */
      DefinitionsServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client);

      ~DefinitionsServiceTestEnvironment();

      /**
       * Returns a new DefinitionsClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the DefinitionsClient.
       */
      DefinitionsClient make_client(
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
      ServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;

      DefinitionsServiceTestEnvironment(
        const DefinitionsServiceTestEnvironment&) = delete;
      DefinitionsServiceTestEnvironment& operator =(
        const DefinitionsServiceTestEnvironment&) = delete;
  };

  inline DefinitionsServiceTestEnvironment::DefinitionsServiceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client)
    : m_container(Beam::Initialize(std::move(service_locator_client),
        Beam::Initialize("1", "Spire Trading Inc.",
          Nexus::Details::get_base_time_zone_table(), DEFAULT_COUNTRIES,
          DEFAULT_CURRENCIES, DEFAULT_DESTINATIONS, DEFAULT_VENUES,
          std::vector<ExchangeRate>(),
          std::vector<Compliance::ComplianceRuleSchema>(), TradingSchedule())),
        &m_server_connection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline DefinitionsServiceTestEnvironment::
      ~DefinitionsServiceTestEnvironment() {
    close();
  }

  inline DefinitionsClient DefinitionsServiceTestEnvironment::make_client(
      Beam::ServiceLocator::ServiceLocatorClientBox service_locator_client) {
    return DefinitionsClient(std::in_place_type<ServiceDefinitionsClient<
      ServiceProtocolClientBuilder>>, ServiceProtocolClientBuilder(
        service_locator_client, std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_definitions_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void DefinitionsServiceTestEnvironment::close() {
    m_container.Close();
  }
}

#endif
