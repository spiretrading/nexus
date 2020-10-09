#ifndef NEXUS_RISK_APPLICATION_DEFINITIONS_HPP
#define NEXUS_RISK_APPLICATION_DEFINITIONS_HPP
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
#include "Nexus/RiskService/RiskClient.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Encapsulates a standard RiskClient used in an application. */
  class ApplicationRiskClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
        Beam::Services::MessageProtocol<
        std::unique_ptr<Beam::Network::TcpSocketChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::LiveTimer>;

      /** Defines the standard RiskClient used for applications. */
      using Client = RiskClient<SessionBuilder>;

      /** Constructs an ApplicationRiskClient. */
      ApplicationRiskClient() = default;

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

      ApplicationRiskClient(const ApplicationRiskClient&) = delete;
      ApplicationRiskClient& operator =(const ApplicationRiskClient&) = delete;
  };

  inline void ApplicationRiskClient::BuildSession(
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

  inline ApplicationRiskClient::Client& ApplicationRiskClient::operator *() {
    return *m_client;
  }

  inline const ApplicationRiskClient::Client&
      ApplicationRiskClient::operator *() const {
    return *m_client;
  }

  inline ApplicationRiskClient::Client* ApplicationRiskClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationRiskClient::Client*
      ApplicationRiskClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationRiskClient::Client* ApplicationRiskClient::Get() {
    return &*m_client;
  }

  inline const ApplicationRiskClient::Client*
      ApplicationRiskClient::Get() const {
    return &*m_client;
  }
}

#endif
