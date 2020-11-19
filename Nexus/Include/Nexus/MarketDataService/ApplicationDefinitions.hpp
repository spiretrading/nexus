#ifndef NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#define NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /** Encapsulates a standard MarketDataClient used in an application. */
  class ApplicationMarketDataClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder = Beam::Services::ZlibSessionBuilder;

      /** Defines the standard MarketDataClient used for applications. */
      using Client = MarketDataClient<SessionBuilder>;

      /**
       * Constructs an ApplicationMarketDataClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      template<typename ServiceLocatorClient>
      explicit ApplicationMarketDataClient(
        Beam::Ref<ServiceLocatorClient> serviceLocatorClient);

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
      std::unique_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Client m_client;

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
  template<typename SessionBuilder, typename ServiceLocatorClient,
    typename Predicate>
  SessionBuilder BuildBasicMarketDataClientSessionBuilder(
      Beam::Ref<ServiceLocatorClient> serviceLocatorClient,
      Predicate&& servicePredicate,
      const std::string& service = RELAY_SERVICE_NAME) {
    return SessionBuilder(Beam::Ref(serviceLocatorClient),
      [servicePredicate = std::forward<Predicate>(servicePredicate), =] {
        return std::make_unique<Beam::Network::TcpSocketChannel>(
          Beam::ServiceLocator::LocateServiceAddresses(*serviceLocatorClient,
          service, servicePredicate));
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  /**
   * Builds a SessionBuilder for a standard MarketDataClient.
   * @param serviceLocatorClient The ServiceLocatorClient used to authenticate
   *        sessions.
   * @param service The name of the service to connect to.
   */
  template<typename ServiceLocatorClient>
  ApplicationMarketDataClient::SessionBuilder
      BuildMarketDataClientSessionBuilder(
      Beam::Ref<ServiceLocatorClient> serviceLocatorClient,
      const std::string& service = RELAY_SERVICE_NAME) {
    return ApplicationMarketDataClient::SessionBuilder(
      Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<Beam::Network::TcpSocketChannel>(
          Beam::ServiceLocator::LocateServiceAddresses(*serviceLocatorClient,
          service));
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  template<typename ServiceLocatorClient>
  ApplicationMarketDataClient::ApplicationMarketDataClient(
      Beam::Ref<ServiceLocatorClient> serviceLocatorClient)
    : m_serviceLocatorClient(
        Beam::ServiceLocator::MakeVirtualServiceLocatorClient(
        serviceLocatorClient.Get())),
      m_client(BuildMarketDataClientSessionBuilder(Beam::Ref(
        *m_serviceLocatorClient))) {}

  inline ApplicationMarketDataClient::Client&
      ApplicationMarketDataClient::operator *() {
    return m_client;
  }

  inline const ApplicationMarketDataClient::Client&
      ApplicationMarketDataClient::operator *() const {
    return m_client;
  }

  inline ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::operator ->() {
    return &m_client;
  }

  inline const ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::operator ->() const {
    return &m_client;
  }

  inline ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::Get() {
    return &m_client;
  }

  inline const ApplicationMarketDataClient::Client*
      ApplicationMarketDataClient::Get() const {
    return &m_client;
  }
}

#endif
