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
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreBox.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClientBox.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

namespace Nexus::MarketDataService::Tests {

  /**
   * Wraps most components needed to run an instance of the MarketDataService
   * with helper functions.
   */
  class MarketDataServiceTestEnvironment {
    public:

      /**
       * Constructs an MarketDataServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param administrationClient The AdministrationClient to use.
       */
      MarketDataServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        AdministrationService::AdministrationClientBox administrationClient);

      /**
       * Constructs an MarketDataServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param administrationClient The AdministrationClient to use.
       * @param dataStore The data store containing market data to test with.
       */
      MarketDataServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        AdministrationService::AdministrationClientBox administrationClient,
        HistoricalDataStoreBox dataStore);

      ~MarketDataServiceTestEnvironment();

      /** Returns the historical data store. */
      HistoricalDataStoreBox& GetDataStore();

      /** Returns the MarketDataRegistry. */
      const MarketDataRegistry& GetRegistry() const;

      /**
       * Publishes an OrderImbalance.
       * @param market The market to publish to.
       * @param orderImbalance The OrderImbalance to publish.
       */
      void Publish(MarketCode market, const OrderImbalance& orderImbalance);

      /**
       * Publishes a BboQuote.
       * @param security The Security to publish to.
       * @param bboQuote The BboQuote to publish.
       */
      void Publish(const Security& security, const BboQuote& bboQuote);

      /**
       * Publishes a BookQuote.
       * @param security The Security to publish to.
       * @param bookQuote The BookQuote to publish.
       */
      void Publish(const Security& security, const BookQuote& bookQuote);

      /**
       * Publishes a MarketQuote.
       * @param security The Security to publish to.
       * @param marketQuote The MarketQuote to publish.
       */
      void Publish(const Security& security, const MarketQuote& marketQuote);

      /**
       * Publishes a TimeAndSale.
       * @param security The Security to publish to.
       * @param timeAndSale The TimeAndSale to publish.
       */
      void Publish(const Security& security, const TimeAndSale& timeAndSale);

      /**
       * Builds a new MarketDataClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the MarketDataClient.
       */
      MarketDataClientBox MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      /**
       * Builds a new MarketDataFeedClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the MarketDataFeedClient.
       */
      MarketDataFeedClientBox MakeFeedClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
          Beam::ServiceLocator::MetaAuthenticationServletAdapter<
            MetaMarketDataRegistryServlet<MarketDataRegistry*,
              HistoricalDataStoreBox,
              AdministrationService::AdministrationClientBox>,
            Beam::ServiceLocator::ServiceLocatorClientBox,
            Beam::NativePointerPolicy>,
          ServerConnection*,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using BaseRegistryServlet = MarketDataRegistryServlet<
        ServiceProtocolServletContainer, MarketDataRegistry*,
        HistoricalDataStoreBox, AdministrationService::AdministrationClientBox>;
      using RegistryServlet =
        Beam::ServiceLocator::AuthenticationServletAdapter<
          ServiceProtocolServletContainer, BaseRegistryServlet*,
          Beam::ServiceLocator::ServiceLocatorClientBox>;
      using FeedServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
          Beam::ServiceLocator::MetaAuthenticationServletAdapter<
            MetaMarketDataFeedServlet<BaseRegistryServlet*>,
            Beam::ServiceLocator::ServiceLocatorClientBox>,
          ServerConnection*,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocator::ServiceLocatorClientBox,
          Beam::Services::MessageProtocol<
            std::unique_ptr<ClientChannel>,
            Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
            Beam::Codecs::NullEncoder>,
          Beam::Threading::TriggerTimer>;
      Beam::ServiceLocator::ServiceLocatorClientBox m_serviceLocatorClient;
      AdministrationService::AdministrationClientBox m_administrationClient;
      MarketDataRegistry m_registry;
      ServerConnection m_serverConnection;
      HistoricalDataStoreBox m_dataStore;
      BaseRegistryServlet m_registryServlet;
      ServiceProtocolServletContainer m_container;
      ServerConnection m_feedServerConnection;
      FeedServiceProtocolServletContainer m_feedContainer;
      Beam::Threading::TriggerTimer m_samplingTimer;
  };

  inline MarketDataServiceTestEnvironment::MarketDataServiceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
    AdministrationService::AdministrationClientBox administrationClient)
    : MarketDataServiceTestEnvironment(std::move(serviceLocatorClient),
        std::move(administrationClient),
        HistoricalDataStoreBox(std::in_place_type<LocalHistoricalDataStore>)) {}

  inline MarketDataServiceTestEnvironment::MarketDataServiceTestEnvironment(
    Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
    AdministrationService::AdministrationClientBox administrationClient,
    HistoricalDataStoreBox dataStore)
    : m_serviceLocatorClient(std::move(serviceLocatorClient)),
      m_administrationClient(std::move(administrationClient)),
      m_dataStore(std::move(dataStore)),
      m_registryServlet(m_administrationClient, &m_registry, m_dataStore),
      m_container(Beam::Initialize(m_serviceLocatorClient,
        &m_registryServlet), &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()),
      m_feedContainer(Beam::Initialize(m_serviceLocatorClient,
        &m_registryServlet), &m_feedServerConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline MarketDataServiceTestEnvironment::~MarketDataServiceTestEnvironment() {
    Close();
  }

  inline HistoricalDataStoreBox&
      MarketDataServiceTestEnvironment::GetDataStore() {
    return m_dataStore;
  }

  inline const MarketDataRegistry&
      MarketDataServiceTestEnvironment::GetRegistry() const {
    return m_registry;
  }

  inline void MarketDataServiceTestEnvironment::Publish(
      MarketCode market, const OrderImbalance& orderImbalance) {
    m_registryServlet.PublishOrderImbalance(
      MarketOrderImbalance{orderImbalance, market}, 0);
  }

  inline void MarketDataServiceTestEnvironment::Publish(
      const Security& security, const BboQuote& bboQuote) {
    m_registryServlet.PublishBboQuote(
      SecurityBboQuote{bboQuote, security}, 0);
  }

  inline void MarketDataServiceTestEnvironment::Publish(
      const Security& security, const BookQuote& bookQuote) {
    m_registryServlet.UpdateBookQuote(
      SecurityBookQuote{bookQuote, security}, 0);
  }

  inline void MarketDataServiceTestEnvironment::Publish(
      const Security& security, const MarketQuote& marketQuote) {
    m_registryServlet.PublishMarketQuote(
      SecurityMarketQuote{marketQuote, security}, 0);
  }

  inline void MarketDataServiceTestEnvironment::Publish(
      const Security& security, const TimeAndSale& timeAndSale) {
    m_registryServlet.PublishTimeAndSale(
      SecurityTimeAndSale{timeAndSale, security}, 0);
  }

  inline MarketDataClientBox MarketDataServiceTestEnvironment::MakeClient(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return MarketDataClientBox(
      std::in_place_type<MarketDataClient<ServiceProtocolClientBuilder>>,
      ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
        std::bind(boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Channel>>(),
          "test_market_data_client", std::ref(m_serverConnection)),
        boost::factory<
          std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline MarketDataFeedClientBox
      MarketDataServiceTestEnvironment::MakeFeedClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    using Client = MarketDataService::MarketDataFeedClient<std::string,
      Beam::Threading::TriggerTimer*, Beam::Services::MessageProtocol<
        ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>,
      Beam::Threading::TriggerTimer>;
    return MarketDataFeedClientBox(std::in_place_type<Client>,
      Beam::Initialize("test_market_data_feed_client", m_feedServerConnection),
      Beam::ServiceLocator::SessionAuthenticator(
        std::move(serviceLocatorClient)),
      &m_samplingTimer, Beam::Initialize());
  }

  inline void MarketDataServiceTestEnvironment::Close() {
    m_feedContainer.Close();
    m_container.Close();
    m_registryServlet.Close();
  }
}

#endif
