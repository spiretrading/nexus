#ifndef NEXUS_ADMINISTRATION_APPLICATION_DEFINITIONS_HPP
#define NEXUS_ADMINISTRATION_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus::AdministrationService {

  /** Encapsulates a standard AdministrationClient used in an application. */
  class ApplicationAdministrationClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
        Beam::Services::MessageProtocol<
        std::unique_ptr<Beam::Network::TcpSocketChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::LiveTimer>;

      /** Defines the standard AdministrationClient used for applications. */
      using Client = AdministrationClient<SessionBuilder>;

      /** Constructs an ApplicationAdministrationClient. */
      ApplicationAdministrationClient() = default;

      /**
       * Builds the session.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      void BuildSession(
        Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
        serviceLocatorClient);

      /** Returns a reference to the Client. */
      Client& operator *();

      /** Returns a reference to the Client. */
      const Client& operator *() const;

      /** Returns a pointer to the Client. */
      Client* operator ->();

      /** Returns a pointer to the Client. */
      const Client* operator ->() const;

      /** Returns a pointer to the Client. */
      Client* Get();

      /** Returns a pointer to the Client. */
      const Client* Get() const;

    private:
      boost::optional<Client> m_client;

      ApplicationAdministrationClient(
        const ApplicationAdministrationClient&) = delete;
      ApplicationAdministrationClient& operator =(
        const ApplicationAdministrationClient&) = delete;
  };

  inline void ApplicationAdministrationClient::BuildSession(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    m_client = boost::none;
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClient, SERVICE_NAME);
    auto sessionBuilder = SessionBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(std::move(sessionBuilder));
  }

  inline ApplicationAdministrationClient::Client&
      ApplicationAdministrationClient::operator *() {
    return *m_client;
  }

  inline const ApplicationAdministrationClient::Client&
      ApplicationAdministrationClient::operator *() const {
    return *m_client;
  }

  inline ApplicationAdministrationClient::Client*
      ApplicationAdministrationClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationAdministrationClient::Client*
      ApplicationAdministrationClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationAdministrationClient::Client*
      ApplicationAdministrationClient::Get() {
    return &*m_client;
  }

  inline const ApplicationAdministrationClient::Client*
      ApplicationAdministrationClient::Get() const {
    return &*m_client;
  }
}

#endif
