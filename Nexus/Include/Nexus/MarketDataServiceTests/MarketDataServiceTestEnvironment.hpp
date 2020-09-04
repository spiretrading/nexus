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
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"
#include "Nexus/MarketDataService/VirtualHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataFeedClient.hpp"

namespace Nexus::MarketDataService::Tests {

  /** Wraps most components needed to run an instance of the MarketDataService
      with helper functions.
   */
  class MarketDataServiceTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs an MarketDataServiceTestEnvironment.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
        \param administrationClient The AdministrationClient to use.
        \param dataStore The data store containing market data to test with.
      */
      MarketDataServiceTestEnvironment(
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient,
        std::shared_ptr<VirtualHistoricalDataStore> dataStore);

      //! Constructs an MarketDataServiceTestEnvironment.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
        \param administrationClient The AdministrationClient to use.
      */
      MarketDataServiceTestEnvironment(
        std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient,
        std::shared_ptr<AdministrationService::VirtualAdministrationClient>
        administrationClient);

      ~MarketDataServiceTestEnvironment();

      //! Returns the historical data store.
      const VirtualHistoricalDataStore& GetDataStore() const;

      //! Returns the MarketDataRegistry.
      const MarketDataRegistry& GetRegistry() const;

      //! Publishes an OrderImbalance.
      /*!
        \param market The market to publish to.
        \param orderImbalance The OrderImbalance to publish.
      */
      void Publish(MarketCode market, const OrderImbalance& orderImbalance);

      //! Publishes a BboQuote.
      /*!
        \param security The Security to publish to.
        \param bboQuote The BboQuote to publish.
      */
      void Publish(const Security& security, const BboQuote& bboQuote);

      //! Publishes a BookQuote.
      /*!
        \param security The Security to publish to.
        \param bookQuote The BookQuote to publish.
      */
      void Publish(const Security& security, const BookQuote& bookQuote);

      //! Publishes a MarketQuote.
      /*!
        \param security The Security to publish to.
        \param marketQuote The MarketQuote to publish.
      */
      void Publish(const Security& security, const MarketQuote& marketQuote);

      //! Publishes a TimeAndSale.
      /*!
        \param security The Security to publish to.
        \param timeAndSale The TimeAndSale to publish.
      */
      void Publish(const Security& security, const TimeAndSale& timeAndSale);

      //! Builds a new MarketDataClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the MarketDataClient.
      */
      std::unique_ptr<VirtualMarketDataClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      //! Builds a new MarketDataFeedClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the MarketDataFeedClient.
      */
      std::unique_ptr<VirtualMarketDataFeedClient> BuildFeedClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaMarketDataRegistryServlet<MarketDataRegistry*,
        VirtualHistoricalDataStore*, std::shared_ptr<AdministrationClient>>,
        ServiceLocatorClient*, Beam::NativePointerPolicy>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using BaseRegistryServlet = MarketDataRegistryServlet<
        ServiceProtocolServletContainer, MarketDataRegistry*,
        VirtualHistoricalDataStore*, std::shared_ptr<AdministrationClient>>;
      using RegistryServlet =
        Beam::ServiceLocator::AuthenticationServletAdapter<
        ServiceProtocolServletContainer, BaseRegistryServlet*,
        ServiceLocatorClient*>;
      using FeedServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaMarketDataFeedServlet<BaseRegistryServlet*>, ServiceLocatorClient*>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      using MarketDataClient = MarketDataService::VirtualMarketDataClient;
      using MarketDataFeedClient =
        MarketDataService::VirtualMarketDataFeedClient;
      std::shared_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::shared_ptr<AdministrationClient> m_administrationClient;
      MarketDataRegistry m_registry;
      ServerConnection m_serverConnection;
      std::shared_ptr<VirtualHistoricalDataStore> m_dataStore;
      BaseRegistryServlet m_registryServlet;
      ServiceProtocolServletContainer m_container;
      ServerConnection m_feedServerConnection;
      FeedServiceProtocolServletContainer m_feedContainer;
      Beam::Threading::TriggerTimer m_samplingTimer;
      std::unique_ptr<MarketDataFeedClient> m_feedClient;
  };

  inline MarketDataServiceTestEnvironment::MarketDataServiceTestEnvironment(
      std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient,
      std::shared_ptr<AdministrationService::VirtualAdministrationClient>
      administrationClient,
      std::shared_ptr<VirtualHistoricalDataStore> dataStore)
      : m_serviceLocatorClient(std::move(serviceLocatorClient)),
        m_administrationClient(std::move(administrationClient)),
        m_dataStore(std::move(dataStore)),
        m_registryServlet(m_administrationClient, &m_registry, &*m_dataStore),
        m_container(Beam::Initialize(m_serviceLocatorClient.get(),
          &m_registryServlet), &m_serverConnection,
          boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()),
        m_feedContainer(Beam::Initialize(m_serviceLocatorClient.get(),
          &m_registryServlet), &m_feedServerConnection,
          boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline MarketDataServiceTestEnvironment::MarketDataServiceTestEnvironment(
    std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
    serviceLocatorClient,
    std::shared_ptr<AdministrationService::VirtualAdministrationClient>
    administrationClient)
    : MarketDataServiceTestEnvironment(std::move(serviceLocatorClient),
        std::move(administrationClient), MakeVirtualHistoricalDataStore(
        std::make_unique<LocalHistoricalDataStore>())) {}

  inline MarketDataServiceTestEnvironment::~MarketDataServiceTestEnvironment() {
    Close();
  }

  inline const VirtualHistoricalDataStore& MarketDataServiceTestEnvironment::
      GetDataStore() const {
    return *m_dataStore;
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

  inline std::unique_ptr<VirtualMarketDataClient>
      MarketDataServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_market_data_client", m_serverConnection);
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<MarketDataService::MarketDataClient<
      ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualMarketDataClient(std::move(client));
  }

  inline std::unique_ptr<VirtualMarketDataFeedClient>
      MarketDataServiceTestEnvironment::BuildFeedClient(
      Beam::Ref<ServiceLocatorClient> serviceLocatorClient) {
    using Client = MarketDataService::MarketDataFeedClient<std::string,
      Beam::Threading::TriggerTimer*, Beam::Services::MessageProtocol<
      ClientChannel, Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
      Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
    auto client = std::make_unique<Client>(Beam::Initialize(
      "test_market_data_feed_client", m_feedServerConnection),
      Beam::ServiceLocator::SessionAuthenticator<ServiceLocatorClient>(
      Beam::Ref(serviceLocatorClient)), &m_samplingTimer, Beam::Initialize());
    return MakeVirtualMarketDataFeedClient(std::move(client));
  }

  inline void MarketDataServiceTestEnvironment::Close() {
    m_feedContainer.Close();
    m_container.Close();
    m_registryServlet.Close();
  }
}

#endif
