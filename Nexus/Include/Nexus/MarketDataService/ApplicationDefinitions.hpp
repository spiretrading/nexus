#ifndef NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#define NEXUS_MARKET_DATA_APPLICATION_DEFINITIONS_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Services/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/MarketDataService/ServiceMarketDataClient.hpp"
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"

namespace Nexus {

  /** A standard MarketDataClient used in an application. */
  class ApplicationMarketDataClient :
      public ServiceMarketDataClient<Beam::ZLibSessionBuilder<>> {
    public:

      /** The type of SessionBuilder used. */
      using SessionBuilder = Beam::ZLibSessionBuilder<>;

      /** The type of client being encapsulated. */
      using Client = ServiceMarketDataClient<SessionBuilder>;

      /**
       * Constructs an ApplicationMarketDataClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      explicit ApplicationMarketDataClient(
        Beam::Ref<SessionBuilder::ServiceLocatorClient> service_locator_client);
  };

  /** Encapsulates a standard MarketDataFeedClient. */
  class ApplicationMarketDataFeedClient :
      public ServiceMarketDataFeedClient<std::string, Beam::LiveTimer,
        Beam::MessageProtocol<Beam::TcpSocketChannel,
          Beam::BinarySender<Beam::SharedBuffer>,
          Beam::SizeDeclarativeEncoder<Beam::ZLibEncoder>>, Beam::LiveTimer> {
    public:

      /** Defines the standard MarketDataFeedClient used for applications. */
      using Client = ServiceMarketDataFeedClient<std::string,
        Beam::LiveTimer, Beam::MessageProtocol<Beam::TcpSocketChannel,
          Beam::BinarySender<Beam::SharedBuffer>,
          Beam::SizeDeclarativeEncoder<Beam::ZLibEncoder>>, Beam::LiveTimer>;

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
      const std::string& service = MARKET_DATA_RELAY_SERVICE_NAME) {
    auto client = Beam::ServiceLocatorClientBox(
      &Beam::FullyDereference(service_locator_client));
    return SessionBuilder(std::move(service_locator_client),
      [=, service_predicate = std::forward<Predicate>(service_predicate)]
          () mutable {
        return std::make_unique<Beam::Network::TcpSocketChannel>(
          Beam::LocateServiceAddresses(
            client, service, service_predicate));
      },
      [] {
        return std::make_unique<Beam::LiveTimer>(
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
        const std::string& service = MARKET_DATA_RELAY_SERVICE_NAME) {
    auto client = Beam::ServiceLocatorClientBox(
      &Beam::FullyDereference(service_locator_client));
    return ApplicationMarketDataClient::SessionBuilder(
      std::move(service_locator_client),
      [=] () mutable {
        return std::make_unique<Beam::Network::TcpSocketChannel>(
          Beam::LocateServiceAddresses(client, service));
      },
      [] {
        return std::make_unique<Beam::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  inline ApplicationMarketDataClient::ApplicationMarketDataClient(
    SessionBuilder::ServiceLocatorClient service_locator_client)
    : m_client(
        make_market_data_client_session_builder(service_locator_client)) {}

  template<typename ServiceLocatorClient>
  inline ApplicationMarketDataFeedClient::ApplicationMarketDataFeedClient(
    ServiceLocatorClient service_locator_client,
    boost::posix_time::time_duration sampling_time, CountryCode country)
    try : m_client([&] {
            auto service = find_market_data_feed_service(
              country, Beam::FullyDereference(service_locator_client));
            if(!service) {
              BOOST_THROW_EXCEPTION(Beam::ConnectException(
                "No market data services available."));
            }
            auto addresses =
              Beam::Parsers::Parse<std::vector<Beam::Network::IpAddress>>(
                boost::get<std::string>(service->GetProperties().At(
                  "addresses")));
            return Client(Beam::Initialize(addresses),
              Beam::SessionAuthenticator(
                std::move(service_locator_client)),
              Beam::Initialize(sampling_time),
              Beam::Initialize(boost::posix_time::seconds(10)));
          }()) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Unable to initialize the market data feed client."));
  }

  template<typename ServiceLocatorClient>
  inline ApplicationMarketDataFeedClient::ApplicationMarketDataFeedClient(
    ServiceLocatorClient service_locator_client,
    boost::posix_time::time_duration sampling_time)
    try : m_client([&] {
            auto services =
              service_locator_client.Locate(MARKET_DATA_FEED_SERVICE_NAME);
            if(services.empty()) {
              BOOST_THROW_EXCEPTION(Beam::ConnectException(
                "No market data services available."));
            }
            auto& service = services.front();
            auto addresses =
              Beam::Parsers::Parse<std::vector<Beam::Network::IpAddress>>(
                boost::get<std::string>(service.GetProperties().At(
                  "addresses")));
            return Client(Beam::Initialize(addresses),
              Beam::SessionAuthenticator(
                std::move(service_locator_client)),
              Beam::Initialize(sampling_time),
              Beam::Initialize(boost::posix_time::seconds(10)));
          }()) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Unable to initialize the market data feed client."));
  }
}

#endif
