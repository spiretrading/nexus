#ifndef NEXUS_ADMINISTRATIONSERVICETESTENVIRONMENT_HPP
#define NEXUS_ADMINISTRATIONSERVICETESTENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
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
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"

namespace Nexus {
namespace AdministrationService {
namespace Tests {

  /*! \class AdministrationServiceTestEnvironment
      \brief Wraps most components needed to run an instance of the
             AdministrationService with helper functions.
   */
  class AdministrationServiceTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs an AdministrationServiceTestEnvironment.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
      */
      AdministrationServiceTestEnvironment(const std::shared_ptr<
        Beam::ServiceLocator::VirtualServiceLocatorClient>&
        serviceLocatorClient);

      ~AdministrationServiceTestEnvironment();

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Builds a new AdministrationClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the AdministrationClient.
      */
      std::unique_ptr<VirtualAdministrationClient> BuildClient(
        Beam::RefType<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

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
        MetaAdministrationServlet<std::shared_ptr<ServiceLocatorClient>,
        LocalAdministrationDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*, Beam::Serialization::BinarySender<
        Beam::IO::SharedBuffer>, Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      LocalAdministrationDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;
  };

  inline AdministrationServiceTestEnvironment::
      AdministrationServiceTestEnvironment(
      const std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>&
      serviceLocatorClient)
      : m_container(Beam::Initialize(serviceLocatorClient,
          Beam::Initialize(serviceLocatorClient,
          MarketDataService::EntitlementDatabase(), &m_dataStore)),
          &m_serverConnection,
          boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    Close();
  }

  inline void AdministrationServiceTestEnvironment::Open() {
    m_container.Open();
  }

  inline void AdministrationServiceTestEnvironment::Close() {
    m_container.Close();
  }

  inline std::unique_ptr<VirtualAdministrationClient>
      AdministrationServiceTestEnvironment::BuildClient(
      Beam::RefType<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    ServiceProtocolClientBuilder builder(Beam::Ref(serviceLocatorClient),
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_administration_client", Beam::Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<AdministrationService::AdministrationClient<
        ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualAdministrationClient(std::move(client));
  }
}
}
}

#endif
