#ifndef NEXUS_ADMINISTRATION_SERVICE_TEST_ENVIRONMENT_HPP
#define NEXUS_ADMINISTRATION_SERVICE_TEST_ENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/TriggerTimer.hpp>
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

namespace Nexus::AdministrationService::Tests {

  /** Wraps most components needed to run an instance of the
      AdministrationService with helper functions.
   */
  class AdministrationServiceTestEnvironment : private boost::noncopyable {
    public:

      //! Constructs an AdministrationServiceTestEnvironment.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient to use.
      */
      AdministrationServiceTestEnvironment(const std::shared_ptr<
        Beam::ServiceLocator::VirtualServiceLocatorClient>&
        serviceLocatorClient);

      ~AdministrationServiceTestEnvironment();

      //! Sets the EntitlementDatabase used by the AdministrationServlet.
      void SetEntitlements(const MarketDataService::EntitlementDatabase&
        entitlements);

      //! Grants an account administrative privileges.
      /*!
        \param account The account to grant administrative privileges to.
      */
      void MakeAdministrator(const Beam::ServiceLocator::DirectoryEntry&
        account);

      //! Builds a new AdministrationClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the AdministrationClient.
      */
      std::unique_ptr<VirtualAdministrationClient> BuildClient(
        Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

      void Close();

    private:
      using ServerConnection =
        Beam::IO::LocalServerConnection<Beam::IO::SharedBuffer>;
      using ClientChannel =
        Beam::IO::LocalClientChannel<Beam::IO::SharedBuffer>;
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;
      using ServiceProtocolServletContainer =
        Beam::Services::ServiceProtocolServletContainer<
        Beam::ServiceLocator::MetaAuthenticationServletAdapter<
        MetaAdministrationServlet<std::shared_ptr<ServiceLocatorClient>,
        LocalAdministrationDataStore*>, std::shared_ptr<ServiceLocatorClient>>,
        ServerConnection*, Beam::Serialization::BinarySender<
        Beam::IO::SharedBuffer>, Beam::Codecs::NullEncoder,
        std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocatorClient, Beam::Services::MessageProtocol<
        std::unique_ptr<ClientChannel>,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Beam::Codecs::NullEncoder>, Beam::Threading::TriggerTimer>;
      std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      LocalAdministrationDataStore m_dataStore;
      ServerConnection m_serverConnection;
      boost::optional<ServiceProtocolServletContainer> m_container;
      boost::optional<MarketDataService::EntitlementDatabase> m_entitlements;
      Beam::ServiceLocator::DirectoryEntry m_globalEntitlementGroup;
  };

  inline AdministrationServiceTestEnvironment::
      AdministrationServiceTestEnvironment(
      const std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>&
      serviceLocatorClient)
      : m_serviceLocatorClient(serviceLocatorClient) {
    if(!m_entitlements) {
      auto entitlementsDirectory = m_serviceLocatorClient->MakeDirectory(
        "entitlements",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
      m_globalEntitlementGroup = m_serviceLocatorClient->MakeDirectory("global",
        entitlementsDirectory);
      auto globalEntitlement = MarketDataService::EntitlementDatabase::Entry();
      globalEntitlement.m_name = "global";
      globalEntitlement.m_groupEntry = m_globalEntitlementGroup;
      auto& marketDatabase = GetDefaultMarketDatabase();
      auto markets = std::vector<MarketCode>();
      for(auto& entry : marketDatabase.GetEntries()) {
        markets.push_back(entry.m_code);
      }
      markets.push_back(MarketCode());
      for(auto& market : markets) {
        globalEntitlement.m_applicability[
          MarketDataService::EntitlementKey(market)].Set(
          MarketDataService::MarketDataType::TIME_AND_SALE);
        globalEntitlement.m_applicability[
          MarketDataService::EntitlementKey(market)].Set(
          MarketDataService::MarketDataType::BOOK_QUOTE);
        globalEntitlement.m_applicability[
          MarketDataService::EntitlementKey(market)].Set(
          MarketDataService::MarketDataType::MARKET_QUOTE);
        globalEntitlement.m_applicability[
          MarketDataService::EntitlementKey(market)].Set(
          MarketDataService::MarketDataType::BBO_QUOTE);
        globalEntitlement.m_applicability[
          MarketDataService::EntitlementKey(market)].Set(
          MarketDataService::MarketDataType::ORDER_IMBALANCE);
      }
      m_entitlements.emplace();
      m_entitlements->Add(globalEntitlement);
    }
    m_container.emplace(Beam::Initialize(m_serviceLocatorClient,
      Beam::Initialize(m_serviceLocatorClient, *m_entitlements, &m_dataStore)),
      &m_serverConnection,
      boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
  }

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    Close();
  }

  inline void AdministrationServiceTestEnvironment::SetEntitlements(
      const MarketDataService::EntitlementDatabase& entitlements) {
    m_entitlements = entitlements;
  }

  inline void AdministrationServiceTestEnvironment::MakeAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto administrators = m_serviceLocatorClient->LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      "administrators");
    m_serviceLocatorClient->Associate(account, administrators);
  }

  inline std::unique_ptr<VirtualAdministrationClient>
      AdministrationServiceTestEnvironment::BuildClient(
      Beam::Ref<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    auto builder = ServiceProtocolClientBuilder(Beam::Ref(serviceLocatorClient),
      [=, serviceLocatorClient = serviceLocatorClient.Get()] {
        if(m_globalEntitlementGroup.m_type !=
            Beam::ServiceLocator::DirectoryEntry::Type::NONE) {
          m_serviceLocatorClient->Associate(serviceLocatorClient->GetAccount(),
            m_globalEntitlementGroup);
        }
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_administration_client", Beam::Ref(m_serverConnection));
      },
      [] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<AdministrationService::AdministrationClient<
      ServiceProtocolClientBuilder>>(builder);
    return MakeVirtualAdministrationClient(std::move(client));
  }

  inline void AdministrationServiceTestEnvironment::Close() {
    if(m_container.is_initialized()) {
      m_container->Close();
      m_container.reset();
    }
  }
}

#endif
