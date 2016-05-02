#ifndef NEXUS_MARKETDATASERVICETESTINSTANCE_HPP
#define NEXUS_MARKETDATASERVICETESTINSTANCE_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/UniquePointerPolicy.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocator/SessionAuthenticator.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Tests {

  /*! \class MarketDataServiceTestInstance
      \brief Wraps most components needed to run an instance of the
             MarketDataService with helper functions.
   */
  class MarketDataServiceTestInstance : private boost::noncopyable {
    public:

      //! The type of ServerConnection.
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;

      //! The type of ServiceLocatorClient used.
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      //! The type of ServiceProtocolServer used for the
      //! MarketDataRegistryServlet.
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaMarketDataRegistryServlet<MarketDataRegistry*,
        LocalHistoricalDataStore*, ServiceLocatorClient>,
        ServiceLocatorClient*, Beam::NativePointerPolicy>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! Used to connect the feed servlet to the registry servlet.
      using BaseRegistryServlet = MarketDataRegistryServlet<
        ServiceProtocolServletContainer, MarketDataRegistry*,
        LocalHistoricalDataStore*, ServiceLocatorClient>;
      using RegistryServlet =
        Beam::ServiceLocator::AuthenticationServletAdapter<
        ServiceProtocolServletContainer, BaseRegistryServlet*,
        ServiceLocatorClient*>;

      //! The type of ServiceProtocolServer used for the
      //! MarketDataFeedServlet.
      using FeedServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaMarketDataFeedServlet<BaseRegistryServlet*>, ServiceLocatorClient*>,
        ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;

      //! The type used to build MarketDataClient sessions.
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! The type of MarketDataClient used.
      using MarketDataClient =
        MarketDataService::MarketDataClient<ServiceProtocolClientBuilder>;

      //! The type of MarketDataFeedClient used.
      using MarketDataFeedClient =
        MarketDataService::MarketDataFeedClient<std::string,
        Beam::Threading::TriggerTimer*,
        Beam::Services::MessageProtocol<ClientChannel,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;

      //! Constructs an MarketDataServiceTestInstance.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
      */
      MarketDataServiceTestInstance(
        std::unique_ptr<ServiceLocatorClient> serviceLocatorClient);

      ~MarketDataServiceTestInstance();

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Returns the historical data store.
      const LocalHistoricalDataStore& GetDataStore() const;

      //! Returns the historical data store.
      LocalHistoricalDataStore& GetDataStore();

      //! Returns the MarketDataRegistry.
      const MarketDataRegistry& GetRegistry() const;

      //! Sets a Security's BBO.
      /*!
        \param security The Security to set.
        \param bbo The <i>security</i>'s BBO.
      */
      void SetBbo(const Security& security, const BboQuote& bbo);

      //! Builds a new MarketDataClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the MarketDataClient.
      */
      std::unique_ptr<MarketDataClient> BuildClient(
        Beam::RefType<ServiceLocatorClient> serviceLocatorClient);

      //! Builds a new MarketDataFeedClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the MarketDataFeedClient.
      */
      std::unique_ptr<MarketDataFeedClient> BuildFeedClient(
        Beam::RefType<ServiceLocatorClient> serviceLocatorClient);

    private:
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      MarketDataRegistry m_registry;
      EntitlementDatabase m_entitlements;
      Beam::ServiceLocator::DirectoryEntry m_globalEntitlementGroup;
      ServerConnection m_serverConnection;
      LocalHistoricalDataStore m_dataStore;
      Beam::DelayPtr<BaseRegistryServlet> m_registryServlet;
      Beam::DelayPtr<ServiceProtocolServletContainer> m_container;
      ServerConnection m_feedServerConnection;
      Beam::DelayPtr<FeedServiceProtocolServletContainer> m_feedContainer;
      Beam::Threading::TriggerTimer m_samplingTimer;
      std::unique_ptr<MarketDataFeedClient> m_feedClient;
  };

  inline MarketDataServiceTestInstance::MarketDataServiceTestInstance(
      std::unique_ptr<ServiceLocatorClient> serviceLocatorClient)
      : m_serviceLocatorClient(std::move(serviceLocatorClient)) {}

  inline MarketDataServiceTestInstance::~MarketDataServiceTestInstance() {
    Close();
  }

  inline void MarketDataServiceTestInstance::Open() {
    auto entitlementsDirectory = m_serviceLocatorClient->MakeDirectory(
      "entitlements", Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    m_globalEntitlementGroup = m_serviceLocatorClient->MakeDirectory("global",
      entitlementsDirectory);
    EntitlementDatabase::Entry globalEntitlement;
    globalEntitlement.m_name = "global";
    globalEntitlement.m_groupEntry = m_globalEntitlementGroup;
    globalEntitlement.m_applicability[EntitlementKey("XNAS", "XNAS")].Set(
      MarketDataType::BBO_QUOTE);
    globalEntitlement.m_applicability[EntitlementKey("XNAS", "XNAS")].Set(
      MarketDataType::TIME_AND_SALE);
    globalEntitlement.m_applicability[EntitlementKey("XNYS", "XNYS")].Set(
      MarketDataType::BBO_QUOTE);
    globalEntitlement.m_applicability[EntitlementKey("XNYS", "XNYS")].Set(
      MarketDataType::TIME_AND_SALE);
    m_entitlements.Add(globalEntitlement);
    m_registryServlet.Initialize(m_entitlements,
      Beam::Ref(*m_serviceLocatorClient), &m_registry, &m_dataStore);
    m_container.Initialize(Beam::Initialize(m_serviceLocatorClient.get(),
      &*m_registryServlet), &m_serverConnection,
      boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
    m_feedContainer.Initialize(Beam::Initialize(m_serviceLocatorClient.get(),
      &*m_registryServlet), &m_feedServerConnection,
      boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
    m_container->Open();
    m_feedContainer->Open();
  }

  inline void MarketDataServiceTestInstance::Close() {
    m_feedContainer.Reset();
    m_container.Reset();
  }

  inline const LocalHistoricalDataStore& MarketDataServiceTestInstance::
      GetDataStore() const {
    return m_dataStore;
  }

  inline LocalHistoricalDataStore& MarketDataServiceTestInstance::
      GetDataStore() {
    return m_dataStore;
  }

  inline const MarketDataRegistry&
      MarketDataServiceTestInstance::GetRegistry() const {
    return m_registry;
  }

  inline void MarketDataServiceTestInstance::SetBbo(const Security& security,
      const BboQuote& bbo) {
    m_registryServlet->PublishBboQuote(SecurityBboQuote(bbo, security), 0);
  }

  inline std::unique_ptr<MarketDataServiceTestInstance::MarketDataClient>
      MarketDataServiceTestInstance::BuildClient(
      Beam::RefType<ServiceLocatorClient> serviceLocatorClient) {
    ServiceProtocolClientBuilder builder(Beam::Ref(serviceLocatorClient),
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_market_data_client", Beam::Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<MarketDataClient>(builder);
    m_serviceLocatorClient->Associate(serviceLocatorClient->GetAccount(),
      m_globalEntitlementGroup);
    return client;
  }

  inline std::unique_ptr<MarketDataServiceTestInstance::MarketDataFeedClient>
      MarketDataServiceTestInstance::BuildFeedClient(
      Beam::RefType<ServiceLocatorClient> serviceLocatorClient) {
    auto client = std::make_unique<MarketDataFeedClient>(Beam::Initialize(
      std::string("test_market_data_feed_client"),
      Beam::Ref(m_feedServerConnection)),
      Beam::ServiceLocator::SessionAuthenticator<ServiceLocatorClient>(
      Beam::Ref(serviceLocatorClient)), &m_samplingTimer, Beam::Initialize());
    return client;
  }
}
}
}

#endif
