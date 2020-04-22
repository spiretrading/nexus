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
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsServlet.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTests.hpp"

namespace Nexus::DefinitionsService::Tests {

  /*! \class DefinitionsServiceTestEnvironment
      \brief Provides DefinitionsService related classes for testing purposes.
   */
  class DefinitionsServiceTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs a DefinitionsServiceTestEnvironment.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
      */
      DefinitionsServiceTestEnvironment(const std::shared_ptr<
        Beam::ServiceLocator::VirtualServiceLocatorClient>&
        serviceLocatorClient);

      ~DefinitionsServiceTestEnvironment();

      //! Builds a new DefinitionsClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the DefinitionsClient.
      */
      std::unique_ptr<VirtualDefinitionsClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      void Open();

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaDefinitionsServlet, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
  };

  inline DefinitionsServiceTestEnvironment::DefinitionsServiceTestEnvironment(
      const std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>&
      serviceLocatorClient)
      : m_container(Beam::Initialize(serviceLocatorClient,
          Beam::Initialize("1", GetDefaultTimeZoneTable(),
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

  inline std::unique_ptr<VirtualDefinitionsClient>
      DefinitionsServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_definitions_client", Beam::Ref(m_serverConnection));
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<DefinitionsService::DefinitionsClient<
      ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualDefinitionsClient(std::move(client));
  }

  inline void DefinitionsServiceTestEnvironment::Open() {
    m_container.Open();
  }

  inline void DefinitionsServiceTestEnvironment::Close() {
    m_container.Close();
  }
}

#endif
