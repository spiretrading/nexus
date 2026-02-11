#ifndef NEXUS_MARKET_DATA_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_MARKET_DATA_SERVICE_TEST_ENVIRONMENT_HPP
#include <memory>
#include <utility>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"
#include "Nexus/MarketDataService/ServiceMarketDataClient.hpp"
#include "Nexus/MarketDataService/ServiceMarketDataFeedClient.hpp"
#include "Nexus/MarketDataServiceTests/TestEnvironmentMarketDataFeedClient.hpp"

namespace Nexus::Tests {

  /**
   * Wraps most components needed to run an instance of the MarketDataService
   * with helper functions.
   */
  class MarketDataServiceTestEnvironment {
    public:

      /**
       * Constructs an MarketDataServiceTestEnvironment.
       * @param service_locator_client The ServiceLocatorClient to use.
       * @param administration_client The AdministrationClient to use.
       */
      MarketDataServiceTestEnvironment(
        Beam::ServiceLocatorClient service_locator_client,
        AdministrationClient administration_client);

      /**
       * Constructs an MarketDataServiceTestEnvironment.
       * @param service_locator_client The ServiceLocatorClient to use.
       * @param administration_client The AdministrationClient to use.
       * @param data_store The data store containing market data to test with.
       */
      MarketDataServiceTestEnvironment(
        Beam::ServiceLocatorClient service_locator_client,
        AdministrationClient administration_client,
        HistoricalDataStore data_store);

      ~MarketDataServiceTestEnvironment();

      /** Returns the historical data store. */
      HistoricalDataStore& get_data_store();

      /** Returns the MarketDataRegistry. */
      const MarketDataRegistry& get_registry() const;

      /** Returns this test environment's MarketDataClient. */
      MarketDataClient& get_registry_client();

      /** Returns this test environment's MarketDataFeedClient. */
      MarketDataFeedClient& get_feed_client();

      /**
       * Returns a new MarketDataClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the MarketDataClient.
       */
      MarketDataClient make_registry_client(
        Beam::Ref<Beam::ServiceLocatorClient> service_locator_client);

      /**
       * Returns a new MarketDataFeedClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the MarketDataClient.
       */
      MarketDataFeedClient make_feed_client(
        Beam::Ref<Beam::ServiceLocatorClient> service_locator_client);

      /**
       * Publishes a BboQuote where only the price is significant and the size
       * is set to 100.
       * @param ticker The ticker whose BboQuote is being updated.
       * @param bid The BboQuote's bid price.
       * @param ask The BboQuote's ask price.
       */
      void update_bbo(const Ticker& ticker, Money bid, Money ask);

      /**
       * Publishes a BboQuote where only the price is significant and both the
       * bid and ask share the same price.
       * @param ticker The ticker whose BboQuote is being updated.
       * @param price The BboQuote's bid and ask price.
       */
      void update_bbo(const Ticker& ticker, Money price);

      void close();

    private:
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          Beam::MetaAuthenticationServletAdapter<
            MetaMarketDataRegistryServlet<
              MarketDataRegistry*, HistoricalDataStore, AdministrationClient>,
            Beam::ServiceLocatorClient, Beam::NativePointerPolicy>,
          Beam::LocalServerConnection*, Beam::BinarySender<Beam::SharedBuffer>,
          Beam::NullEncoder, std::shared_ptr<Beam::TriggerTimer>>;
      using BaseRegistryServlet = MarketDataRegistryServlet<
        ServiceProtocolServletContainer, MarketDataRegistry*,
        HistoricalDataStore, AdministrationClient>;
      using RegistryServlet = Beam::AuthenticationServletAdapter<
        ServiceProtocolServletContainer, BaseRegistryServlet*,
        Beam::ServiceLocatorClient>;
      using FeedServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          Beam::MetaAuthenticationServletAdapter<
            MetaMarketDataFeedServlet<BaseRegistryServlet*>,
            Beam::ServiceLocatorClient>, Beam::LocalServerConnection*,
          Beam::BinarySender<Beam::SharedBuffer>, Beam::NullEncoder,
          std::shared_ptr<Beam::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocatorClient,
          Beam::MessageProtocol<std::unique_ptr<Beam::LocalClientChannel>,
            Beam::BinarySender<Beam::SharedBuffer>, Beam::NullEncoder>,
          Beam::TriggerTimer>;
      Beam::ServiceLocatorClient m_service_locator_client;
      AdministrationClient m_administration_client;
      MarketDataRegistry m_registry;
      Beam::LocalServerConnection m_server_connection;
      HistoricalDataStore m_data_store;
      BaseRegistryServlet m_registry_servlet;
      ServiceProtocolServletContainer m_container;
      Beam::LocalServerConnection m_feed_server_connection;
      FeedServiceProtocolServletContainer m_feed_container;
      MarketDataClient m_registry_client;
      MarketDataFeedClient m_feed_client;
  };

  inline MarketDataServiceTestEnvironment
        make_market_data_service_test_environment(
      Beam::Tests::ServiceLocatorTestEnvironment& service_locator_environment,
      Tests::AdministrationServiceTestEnvironment& administration_environment) {
    auto& root_client = service_locator_environment.get_root();
    auto services_root = Beam::load_or_create_directory(
      root_client, "services", Beam::DirectoryEntry::STAR_DIRECTORY);
    auto market_data_account =
      root_client.make_account("market_data_service", "", services_root);
    root_client.store(market_data_account, Beam::DirectoryEntry::STAR_DIRECTORY,
      Beam::Permissions(~0));
    administration_environment.make_administrator(market_data_account);
    auto service_locator_client =
      service_locator_environment.make_client("market_data_service", "");
    auto administration_client =
      administration_environment.make_client(Beam::Ref(service_locator_client));
    return MarketDataServiceTestEnvironment(
      std::move(service_locator_client), std::move(administration_client));
  }

  /**
   * Makes a MarketDataClient connected to a MarketDataTestEnvironment with all
   * available entitlements.
   * @param service_locator_environment The service locator environment managing
   *        the test account.
   * @param administration_environment The administration service managing
   *        entitlements.
   * @param market_data_environment The market data test environment to
   *        connect the MarketDataClient to.
   * @param account_name The name of the account.
   */
  inline MarketDataClient make_market_data_client(
      Beam::Tests::ServiceLocatorTestEnvironment& service_locator_environment,
      Tests::AdministrationServiceTestEnvironment& administration_environment,
      MarketDataServiceTestEnvironment& market_data_environment,
      const std::string& account_name) {
    service_locator_environment.get_root().make_account(
      account_name, "", Beam::DirectoryEntry::STAR_DIRECTORY);
    auto service_locator =
      service_locator_environment.make_client(account_name, "");
    grant_all_entitlements(
      administration_environment, service_locator.get_account());
    return market_data_environment.make_registry_client(
      Beam::Ref(service_locator));
  }

  inline MarketDataServiceTestEnvironment::MarketDataServiceTestEnvironment(
    Beam::ServiceLocatorClient service_locator_client,
    AdministrationClient administration_client)
    : MarketDataServiceTestEnvironment(
        std::move(service_locator_client), std::move(administration_client),
        HistoricalDataStore(std::in_place_type<LocalHistoricalDataStore>)) {}

  inline MarketDataServiceTestEnvironment::MarketDataServiceTestEnvironment(
    Beam::ServiceLocatorClient service_locator_client,
    AdministrationClient administration_client, HistoricalDataStore data_store)
    : m_service_locator_client(std::move(service_locator_client)),
      m_administration_client(std::move(administration_client)),
      m_registry(DEFAULT_VENUES, get_default_time_zone_database()),
      m_data_store(std::move(data_store)),
      m_registry_servlet(m_administration_client, &m_registry, m_data_store),
      m_container(Beam::init(m_service_locator_client, &m_registry_servlet),
        &m_server_connection,
        boost::factory<std::shared_ptr<Beam::TriggerTimer>>()),
      m_feed_container(Beam::init(
          m_service_locator_client, &m_registry_servlet),
        &m_feed_server_connection,
        boost::factory<std::shared_ptr<Beam::TriggerTimer>>()),
      m_registry_client(
        make_registry_client(Beam::Ref(m_service_locator_client))),
      m_feed_client(make_feed_client(Beam::Ref(m_service_locator_client))) {}

  inline MarketDataServiceTestEnvironment::~MarketDataServiceTestEnvironment() {
    close();
  }

  inline HistoricalDataStore&
      MarketDataServiceTestEnvironment::get_data_store() {
    return m_data_store;
  }

  inline const MarketDataRegistry&
      MarketDataServiceTestEnvironment::get_registry() const {
    return m_registry;
  }

  inline MarketDataClient&
      MarketDataServiceTestEnvironment::get_registry_client() {
    return m_registry_client;
  }

  inline MarketDataFeedClient&
      MarketDataServiceTestEnvironment::get_feed_client() {
    return m_feed_client;
  }

  inline MarketDataClient
      MarketDataServiceTestEnvironment::make_registry_client(
        Beam::Ref<Beam::ServiceLocatorClient> service_locator_client) {
    return MarketDataClient(
      std::in_place_type<ServiceMarketDataClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(Beam::Ref(service_locator_client),
        std::bind_front(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_market_data_client", std::ref(m_server_connection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline MarketDataFeedClient
      MarketDataServiceTestEnvironment::make_feed_client(
        Beam::Ref<Beam::ServiceLocatorClient> service_locator_client) {
    using Client = ServiceMarketDataFeedClient<
      std::string, std::shared_ptr<Beam::TriggerTimer>,
      Beam::MessageProtocol<std::unique_ptr<Beam::LocalClientChannel>,
        Beam::BinarySender<Beam::SharedBuffer>, Beam::NullEncoder>,
        Beam::TriggerTimer>;
    auto sampling_timer = std::make_shared<Beam::TriggerTimer>();
    auto service_client = MarketDataFeedClient(std::in_place_type<Client>,
      std::make_unique<Beam::LocalClientChannel>(
        "test_market_data_feed_client", m_feed_server_connection),
      Beam::SessionAuthenticator(Beam::Ref(service_locator_client)),
      sampling_timer, Beam::init());
    return MarketDataFeedClient(
      std::in_place_type<TestEnvironmentMarketDataFeedClient>,
      std::move(service_client), std::move(sampling_timer));
  }

  inline void MarketDataServiceTestEnvironment::update_bbo(
      const Ticker& ticker, Money bid, Money ask) {
    get_feed_client().publish(
      TickerBboQuote(BboQuote(make_bid(bid, 100), make_ask(ask, 100),
        boost::posix_time::ptime()), ticker));
  }

  inline void MarketDataServiceTestEnvironment::update_bbo(
      const Ticker& ticker, Money price) {
    update_bbo(ticker, price, price);
  }

  inline void MarketDataServiceTestEnvironment::close() {
    m_feed_container.close();
    m_container.close();
    m_registry_servlet.close();
  }
}

#endif
