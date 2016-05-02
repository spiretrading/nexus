#ifndef NEXUS_ADMINISTRATIONSERVICETESTINSTANCE_HPP
#define NEXUS_ADMINISTRATIONSERVICETESTINSTANCE_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"

namespace Nexus {
namespace AdministrationService {
namespace Tests {

  /*! \class AdministrationServiceTestInstance
      \brief Wraps most components needed to run an instance of the
             AdministrationService with helper functions.
   */
  class AdministrationServiceTestInstance : private boost::noncopyable {
    public:

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceLocatorClient used.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaAdministrationServlet<std::shared_ptr<ServiceLocatorClient>,
        LocalAdministrationDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*, Beam::Serialization::BinarySender<
        Beam::IO::SharedBuffer>, Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type used to build AdministrationClient sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of AdministrationClient used.
      using AdministrationClient = AdministrationService::AdministrationClient<
        ServiceProtocolClientBuilder>;

      //! Constructs an AdministrationServiceTestInstance.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
      */
      AdministrationServiceTestInstance(
        const std::shared_ptr<ServiceLocatorClient>& serviceLocatorClient);

      ~AdministrationServiceTestInstance();

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Builds a new AdministrationClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the AdministrationClient.
      */
      std::unique_ptr<AdministrationClient> BuildClient(
        Beam::RefType<ServiceLocatorClient> serviceLocatorClient);

    private:
      LocalAdministrationDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
  };

  inline AdministrationServiceTestInstance::AdministrationServiceTestInstance(
      const std::shared_ptr<ServiceLocatorClient>& serviceLocatorClient)
      : m_container(Beam::Initialize(serviceLocatorClient,
          Beam::Initialize(serviceLocatorClient,
          MarketDataService::EntitlementDatabase(), &m_dataStore)),
          &m_serverConnection,
          boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline AdministrationServiceTestInstance::
      ~AdministrationServiceTestInstance() {
    Close();
  }

  inline void AdministrationServiceTestInstance::Open() {
    m_container.Open();
  }

  inline void AdministrationServiceTestInstance::Close() {
    m_container.Close();
  }

  inline std::unique_ptr<AdministrationServiceTestInstance::
      AdministrationClient> AdministrationServiceTestInstance::BuildClient(
      Beam::RefType<ServiceLocatorClient> serviceLocatorClient) {
    ServiceProtocolClientBuilder builder(Beam::Ref(serviceLocatorClient),
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_administration_client", Beam::Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<AdministrationClient>(builder);
    return client;
  }
}
}
}

#endif
