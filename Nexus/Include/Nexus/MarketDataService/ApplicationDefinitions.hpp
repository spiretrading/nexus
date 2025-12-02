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
      template<Beam::IsServiceLocatorClient C>
      ApplicationMarketDataFeedClient(Beam::Ref<C> service_locator_client,
        boost::posix_time::time_duration sampling_time, CountryCode country);

      /**
       * Constructs an ApplicationMarketDataFeedClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate sessions.
       * @param sampling_time The duration used for message sampling.
       */
      template<Beam::IsServiceLocatorClient C>
      ApplicationMarketDataFeedClient(Beam::Ref<C> service_locator_client,
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
      Beam::Ref<typename SessionBuilder::ServiceLocatorClient>
        service_locator_client, Predicate&& service_predicate,
      const std::string& service = MARKET_DATA_RELAY_SERVICE_NAME) {
    return SessionBuilder(Beam::Ref(service_locator_client),
      [=, client = service_locator_client.get(),
          service_predicate = std::forward<Predicate>(service_predicate)] ()
            mutable {
        return std::make_unique<Beam::TcpSocketChannel>(
          Beam::locate_service_addresses(*client, service, service_predicate));
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
      make_market_data_client_session_builder(Beam::Ref<
        ApplicationMarketDataClient::SessionBuilder::ServiceLocatorClient>
          service_locator_client,
        const std::string& service = MARKET_DATA_RELAY_SERVICE_NAME) {
    return ApplicationMarketDataClient::SessionBuilder(
      Beam::Ref(service_locator_client),
      [=, client = service_locator_client.get()] () mutable {
        return std::make_unique<Beam::TcpSocketChannel>(
          Beam::locate_service_addresses(*client, service));
      },
      [] {
        return std::make_unique<Beam::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  inline ApplicationMarketDataClient::ApplicationMarketDataClient(
    Beam::Ref<SessionBuilder::ServiceLocatorClient> service_locator_client)
    : ServiceMarketDataClient<Beam::ZLibSessionBuilder<>>(
        make_market_data_client_session_builder(
          Beam::Ref(service_locator_client))) {}

  template<Beam::IsServiceLocatorClient C>
  inline ApplicationMarketDataFeedClient::ApplicationMarketDataFeedClient(
    Beam::Ref<C> service_locator_client,
    boost::posix_time::time_duration sampling_time, CountryCode country)
    : ServiceMarketDataFeedClient<std::string, Beam::LiveTimer,
        Beam::MessageProtocol<Beam::TcpSocketChannel,
          Beam::BinarySender<Beam::SharedBuffer>,
          Beam::SizeDeclarativeEncoder<Beam::ZLibEncoder>>, Beam::LiveTimer>(
            [&] {
              try {
                auto service =
                  find_market_data_feed_service(country, *service_locator_client);
                if(!service) {
                  boost::throw_with_location(Beam::ConnectException(
                    "No market data services available."));
                }
                auto addresses = Beam::parse<std::vector<Beam::IpAddress>>(
                  boost::get<std::string>(
                    service->get_properties().at("addresses")));
                return Beam::init(addresses);
              } catch(const std::exception&) {
                std::throw_with_nested(Beam::ConnectException(
                  "Failed to connect to the market data server."));
              }
            }(),
            Beam::SessionAuthenticator(Beam::Ref(service_locator_client)),
            Beam::init(sampling_time),
            Beam::init(boost::posix_time::seconds(10))) {}

  template<Beam::IsServiceLocatorClient C>
  inline ApplicationMarketDataFeedClient::ApplicationMarketDataFeedClient(
    Beam::Ref<C> service_locator_client,
    boost::posix_time::time_duration sampling_time)
    : ServiceMarketDataFeedClient<std::string, Beam::LiveTimer,
        Beam::MessageProtocol<Beam::TcpSocketChannel,
          Beam::BinarySender<Beam::SharedBuffer>,
          Beam::SizeDeclarativeEncoder<Beam::ZLibEncoder>>, Beam::LiveTimer>(
            [&] {
              try {
                auto services =
                  service_locator_client->locate(MARKET_DATA_FEED_SERVICE_NAME);
                if(services.empty()) {
                  boost::throw_with_location(Beam::ConnectException(
                    "No market data services available."));
                }
                auto& service = services.front();
                auto addresses = Beam::parse<std::vector<Beam::IpAddress>>(
                  boost::get<std::string>(
                    service.get_properties().at("addresses")));
                return Beam::init(addresses);
              } catch(const std::exception&) {
                std::throw_with_nested(Beam::ConnectException(
                  "Failed to connect to the market data server."));
              }
            }(),
            Beam::SessionAuthenticator(Beam::Ref(service_locator_client)),
            Beam::init(sampling_time),
            Beam::init(boost::posix_time::seconds(10))) {}
}

#endif
