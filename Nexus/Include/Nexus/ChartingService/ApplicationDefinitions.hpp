#ifndef NEXUS_CHARTING_APPLICATION_DEFINITIONS_HPP
#define NEXUS_CHARTING_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
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
#include "Nexus/ChartingService/ChartingClient.hpp"
#include "Nexus/ChartingService/ChartingService.hpp"

namespace Nexus::ChartingService {

  /** Encapsulates a standard ChartingClient used in an application. */
  class ApplicationChartingClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
        Beam::Services::MessageProtocol<
        std::unique_ptr<Beam::Network::TcpSocketChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::SizeDeclarativeEncoder<Beam::Codecs::ZLibEncoder>>,
        Beam::Threading::LiveTimer>;

      /** Defines the standard ChartingClient used for applications. */
      using Client = ChartingClient<SessionBuilder>;

      /** Constructs an ApplicationChartingClient. */
      ApplicationChartingClient() = default;

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

      ApplicationChartingClient(const ApplicationChartingClient&) = delete;
      ApplicationChartingClient& operator =(
        const ApplicationChartingClient&) = delete;
  };

  inline void ApplicationChartingClient::BuildSession(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    m_client = boost::none;
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClient, SERVICE_NAME);
    auto sessionBuilder = SessionBuilder(Beam::Ref(serviceLocatorClient),
      [=] () mutable {
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(std::move(sessionBuilder));
  }

  inline ApplicationChartingClient::Client&
      ApplicationChartingClient::operator *() {
    return *m_client;
  }

  inline const ApplicationChartingClient::Client&
      ApplicationChartingClient::operator *() const {
    return *m_client;
  }

  inline ApplicationChartingClient::Client*
      ApplicationChartingClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationChartingClient::Client*
      ApplicationChartingClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationChartingClient::Client* ApplicationChartingClient::Get() {
    return &*m_client;
  }

  inline const ApplicationChartingClient::Client*
      ApplicationChartingClient::Get() const {
    return &*m_client;
  }
}

#endif
