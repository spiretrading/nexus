#ifndef NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#include <optional>
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
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {
namespace Details {
  using DefinitionsClientSessionBuilder =
    Beam::Services::AuthenticatedServiceProtocolClientBuilder<
    Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
    Beam::Services::MessageProtocol<std::unique_ptr<
    Beam::Network::TcpSocketChannel>, Beam::Serialization::BinarySender<
    Beam::IO::SharedBuffer>, Beam::Codecs::NullEncoder>,
    Beam::Threading::LiveTimer>;
}

  /** Encapsulates a standard DefinitionsClient used in an application. */
  class ApplicationDefinitionsClient : private boost::noncopyable {
    public:

      /** Defines the standard DefinitionsClient used for applications. */
      using Client =
        DefinitionsClient<Details::DefinitionsClientSessionBuilder>;

      /** Constructs an ApplicationDefinitionsClient. */
      ApplicationDefinitionsClient() = default;

      /**
       * Builds the session.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      void BuildSession(Beam::Ref<Beam::ServiceLocator::
        ApplicationServiceLocatorClient::Client> serviceLocatorClient);

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
      std::optional<Client> m_client;
  };

  inline void ApplicationDefinitionsClient::BuildSession(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    if(m_client.has_value()) {
      m_client->Close();
      m_client = std::nullopt;
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, SERVICE_NAME);
    auto delay = false;
    auto sessionBuilder = Details::DefinitionsClientSessionBuilder(
      Beam::Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          auto delayTimer = Beam::Threading::LiveTimer(
            boost::posix_time::seconds(3));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(sessionBuilder);
  }

  inline ApplicationDefinitionsClient::Client&
      ApplicationDefinitionsClient::operator *() {
    return *m_client;
  }

  inline const ApplicationDefinitionsClient::Client&
      ApplicationDefinitionsClient::operator *() const {
    return *m_client;
  }

  inline ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::Get() {
    return &*m_client;
  }

  inline const ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::Get() const {
    return &*m_client;
  }
}

#endif
