#ifndef NEXUS_ADMINISTRATIONSERVICETESTENVIRONMENT_HPP
#define NEXUS_ADMINISTRATIONSERVICETESTENVIRONMENT_HPP
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
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
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"

namespace Nexus {
namespace AdministrationService {
namespace Tests {

  /*! \class AdministrationServiceTestEnvironment
      \brief Wraps most components needed to run an instance of the
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

      //! Opens the servlet.
      void Open();

      //! Closes the servlet.
      void Close();

      //! Builds a new AdministrationClient.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the AdministrationClient.
      */
      std::unique_ptr<VirtualAdministrationClient> BuildClient(
        Beam::RefType<Beam::ServiceLocator::VirtualServiceLocatorClient>
        serviceLocatorClient);

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
      Beam::ServiceLocator::DirectoryEntry m_globalEntitlementGroup;
  };

  inline AdministrationServiceTestEnvironment::
      AdministrationServiceTestEnvironment(
      const std::shared_ptr<Beam::ServiceLocator::VirtualServiceLocatorClient>&
      serviceLocatorClient)
      : m_serviceLocatorClient{serviceLocatorClient} {}

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    Close();
  }

  inline void AdministrationServiceTestEnvironment::Open() {
    m_serviceLocatorClient->Open();
    auto entitlementsDirectory = m_serviceLocatorClient->MakeDirectory(
      "entitlements",
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    m_globalEntitlementGroup = m_serviceLocatorClient->MakeDirectory("global",
      entitlementsDirectory);
    MarketDataService::EntitlementDatabase::Entry globalEntitlement;
    globalEntitlement.m_name = "global";
    globalEntitlement.m_groupEntry = m_globalEntitlementGroup;
    auto& marketDatabase = GetDefaultMarketDatabase();
    for(auto& market : marketDatabase.GetEntries()) {
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey{market.m_code}].Set(
        MarketDataService::MarketDataType::TIME_AND_SALE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey{market.m_code}].Set(
        MarketDataService::MarketDataType::BOOK_QUOTE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey{market.m_code}].Set(
        MarketDataService::MarketDataType::MARKET_QUOTE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey{market.m_code}].Set(
        MarketDataService::MarketDataType::BBO_QUOTE);
      globalEntitlement.m_applicability[
        MarketDataService::EntitlementKey{market.m_code}].Set(
        MarketDataService::MarketDataType::ORDER_IMBALANCE);
    }
    MarketDataService::EntitlementDatabase entitlementDatabase;
    entitlementDatabase.Add(globalEntitlement);
    m_container.emplace(Beam::Initialize(m_serviceLocatorClient,
      Beam::Initialize(m_serviceLocatorClient, entitlementDatabase,
      &m_dataStore)), &m_serverConnection,
      boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>());
    m_container->Open();
  }

  inline void AdministrationServiceTestEnvironment::Close() {
    m_container->Close();
    m_container.reset();
  }

  inline std::unique_ptr<VirtualAdministrationClient>
      AdministrationServiceTestEnvironment::BuildClient(
      Beam::RefType<Beam::ServiceLocator::VirtualServiceLocatorClient>
      serviceLocatorClient) {
    ServiceProtocolClientBuilder builder(Beam::Ref(serviceLocatorClient),
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Channel>(
          "test_administration_client", Beam::Ref(m_serverConnection));
      },
      [&] {
        return std::make_unique<ServiceProtocolClientBuilder::Timer>();
      });
    auto client = std::make_unique<AdministrationService::AdministrationClient<
        ServiceProtocolClientBuilder>>(builder);
    m_serviceLocatorClient->Associate(serviceLocatorClient->GetAccount(),
      m_globalEntitlementGroup);
    return MakeVirtualAdministrationClient(std::move(client));
  }
}
}
}

#endif
