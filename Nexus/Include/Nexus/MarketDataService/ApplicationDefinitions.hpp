#ifndef NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#define NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Services/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/MarketDataService/ServiceMarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus::MarketDataService {

  /** Encapsulates a standard MarketDataClient used in an application. */
  class ApplicationMarketDataClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder = Beam::Services::ZLibSessionBuilder<>;

      /** Defines the standard MarketDataClient used for applications. */
      using Client = ServiceMarketDataClient<SessionBuilder>;

      /**
       * Constructs an ApplicationMarketDataClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      explicit ApplicationMarketDataClient(
        SessionBuilder::ServiceLocatorClient service_locator_client);

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
      Client m_client;

      ApplicationMarketDataClient(const ApplicationMarketDataClient&) = delete;
      ApplicationMarketDataClient& operator =(
        const ApplicationMarketDataClient&) = delete;
  };

  /** Encapsulates a standard MarketDataFeedClient. */
  class ApplicationMarketDataFeedClient {
    public:

      /** Defines the standard MarketDataFeedClient used for applications. */
      using Client = MarketDataFeedClient<std::string,
        Beam::Threading::LiveTimer, Beam::Services::MessageProtocol<
          Beam::Network::TcpSocketChannel,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::SizeDeclarativeEncoder<Beam::Codecs::ZLibEncoder>>,
        Beam::Threading::LiveTimer>;

      /**
       * Constructs an ApplicationMarketDataFeedClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate sessions.
       * @param sampling_time The duration used for message sampling.
       * @param country The country to publish market data for, used to
       *        determine which market data registry server to connect to.
       */
      template<typename ServiceLocatorClient>
      ApplicationMarketDataFeedClient(
        ServiceLocatorClient service_locator_client,
        boost::posix_time::time_duration sampling_time, CountryCode country);

      /**
       * Constructs an ApplicationMarketDataFeedClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate sessions.
       * @param sampling_time The duration used for message sampling.
       */
      template<typename ServiceLocatorClient>
      ApplicationMarketDataFeedClient(
        ServiceLocatorClient service_locator_client,
        boost::posix_time::time_duration sampling_time);

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
      Client m_client;

      ApplicationMarketDataFeedClient(
        const ApplicationMarketDataFeedClient&) = delete;
      ApplicationMarketDataFeedClient& operator =(
        const ApplicationMarketDataFeedClient&) = delete;
  };

  /**
   * Makes a SessionBuilder for a standard MarketDataClient.
   * @param service_locator_client The ServiceLocatorClient used to authenticate
   *        sessions.
   * @param service_predicate The function used to match an appropriate
   *        ServiceEntry.
   * @param service The name of the service to connect to.
   */
  template<typename SessionBuilder, typename Predicate>
  SessionBuilder make_basic_market_data_client_session_builder(
      typename SessionBuilder::ServiceLocatorClient service_locator_client,
      Predicate&& service_predicate,
      const std::string& service = RELAY_SERVICE_NAME) {
    auto client = Beam::ServiceLocator::ServiceLocatorClientBox(
      &Beam::FullyDereference(service_locator_client));
    return SessionBuilder(std::move(service_locator_client),
      [=, service_predicate = std::forward<Predicate>(service_predicate)]
          () mutable {
        return std::make_unique<Beam::Network::TcpSocketChannel>(
          Beam::ServiceLocator::LocateServiceAddresses(
            client, service, service_predicate));
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  /**
   * Returns a SessionBuilder for a standard MarketDataClient.
   * @param service_locator_client The ServiceLocatorClient used to authenticate
   *        sessions.
   * @param service The name of the service to connect to.
   */
  inline ApplicationMarketDataClient::SessionBuilder
      make_market_data_client_session_builder(
        ApplicationMarketDataClient::SessionBuilder::ServiceLocatorClient
          service_locator_client,
        const std::string& service = RELAY_SERVICE_NAME) {
    auto client = Beam::ServiceLocator::ServiceLocatorClientBox(
      &Beam::FullyDereference(service_locator_client));
    return ApplicationMarketDataClient::SessionBuilder(
      std::move(service_locator_client),
      [=] () mutable {
        return std::make_unique<Beam::Network::TcpSocketChannel>(
          Beam::ServiceLocator::LocateServiceAddresses(client, service));
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  inline ApplicationMarketDataClient::ApplicationMarketDataClient(
    SessionBuilder::ServiceLocatorClient service_locator_client)
    : m_client(
        make_market_data_client_session_builder(service_locator_client)) {}

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

  template<typename ServiceLocatorClient>
  inline ApplicationMarketDataFeedClient::ApplicationMarketDataFeedClient(
    ServiceLocatorClient service_locator_client,
    boost::posix_time::time_duration sampling_time, CountryCode country)
    try : m_client([&] {
            auto service = find_market_data_feed_service(
              country, Beam::FullyDereference(service_locator_client));
            if(!service) {
              BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
                "No market data services available."));
            }
            auto addresses =
              Beam::Parsers::Parse<std::vector<Beam::Network::IpAddress>>(
                boost::get<std::string>(service->GetProperties().At(
                  "addresses")));
            return Client(Beam::Initialize(addresses),
              Beam::ServiceLocator::SessionAuthenticator(
                std::move(service_locator_client)),
              Beam::Initialize(sampling_time),
              Beam::Initialize(boost::posix_time::seconds(10)));
          }()) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Unable to initialize the market data feed client."));
  }

  template<typename ServiceLocatorClient>
  inline ApplicationMarketDataFeedClient::ApplicationMarketDataFeedClient(
    ServiceLocatorClient service_locator_client,
    boost::posix_time::time_duration sampling_time)
    try : m_client([&] {
            auto services = service_locator_client.Locate(FEED_SERVICE_NAME);
            if(services.empty()) {
              BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
                "No market data services available."));
            }
            auto& service = services.front();
            auto addresses =
              Beam::Parsers::Parse<std::vector<Beam::Network::IpAddress>>(
                boost::get<std::string>(service.GetProperties().At(
                  "addresses")));
            return Client(Beam::Initialize(addresses),
              Beam::ServiceLocator::SessionAuthenticator(
                std::move(service_locator_client)),
              Beam::Initialize(sampling_time),
              Beam::Initialize(boost::posix_time::seconds(10)));
          }()) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Unable to initialize the market data feed client."));
  }

  inline ApplicationMarketDataFeedClient::Client&
      ApplicationMarketDataFeedClient::operator *() {
    return m_client;
  }

  inline const ApplicationMarketDataFeedClient::Client&
      ApplicationMarketDataFeedClient::operator *() const {
    return m_client;
  }

  inline ApplicationMarketDataFeedClient::Client*
      ApplicationMarketDataFeedClient::operator ->() {
    return &m_client;
  }

  inline const ApplicationMarketDataFeedClient::Client*
      ApplicationMarketDataFeedClient::operator ->() const {
    return &m_client;
  }

  inline ApplicationMarketDataFeedClient::Client*
      ApplicationMarketDataFeedClient::Get() {
    return &m_client;
  }

  inline const ApplicationMarketDataFeedClient::Client*
      ApplicationMarketDataFeedClient::Get() const {
    return &m_client;
  }
}

#endif
