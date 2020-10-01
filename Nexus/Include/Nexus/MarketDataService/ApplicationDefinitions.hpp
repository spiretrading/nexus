#ifndef NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#define NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
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
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /** Encapsulates a standard MarketDataClient used in an application. */
  class ApplicationMarketDataClient {
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

      /** Defines the standard MarketDataClient used for applications. */
      using Client = MarketDataClient<SessionBuilder>;

      /** Constructs an ApplicationMarketDataClient. */
      ApplicationMarketDataClient() = default;

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

      ApplicationMarketDataClient(const ApplicationMarketDataClient&) = delete;
      ApplicationMarketDataClient& operator =(
        const ApplicationMarketDataClient&) = delete;
  };

  /**
   * Builds a SessionBuilder for a standard MarketDataClient.
   * @param serviceLocatorClient The ServiceLocatorClient used to authenticate
   *        sessions.
   * @param servicePredicate The function used to match an appropriate
   *        ServiceEntry.
   * @param service The name of the service to connect to.
   */
  template<typename SessionBuilder, typename Predicate>
  SessionBuilder BuildBasicMarketDataClientSessionBuilder(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient, const Predicate& servicePredicate,
      const std::string& service = RELAY_SERVICE_NAME) {
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClient, service, servicePredicate);
    auto sessionBuilder = SessionBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    return sessionBuilder;
  }

  /**
   * Builds a SessionBuilder for a standard MarketDataClient.
   * @param serviceLocatorClient The ServiceLocatorClient used to authenticate
   *        sessions.
   * @param service The name of the service to connect to.
   */
  inline ApplicationMarketDataClient::SessionBuilder
      BuildMarketDataClientSessionBuilder(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient, const std::string& service = RELAY_SERVICE_NAME) {
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClient, service);
    auto sessionBuilder = ApplicationMarketDataClient::SessionBuilder(
      Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    return sessionBuilder;
  }

  inline void ApplicationMarketDataClient::BuildSession(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    m_client = boost::none;
    m_client.emplace(BuildMarketDataClientSessionBuilder(
      Beam::Ref(serviceLocatorClient)));
  }

  inline ApplicationMarketDataClient::Client&
      ApplicationMarketDataClient::operator *() {
    return *m_client;
  }

  inline const ApplicationMarketDataClient::Client&
      ApplicationMarketDataClient::operator *() const {
    return *m_client;
  }

  inline ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::Get() {
    return &*m_client;
  }

  inline const ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::Get() const {
    return &*m_client;
  }
}

#endif
