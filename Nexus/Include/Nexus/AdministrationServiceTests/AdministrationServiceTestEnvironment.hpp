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
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTests.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

namespace Nexus::AdministrationService::Tests {

  /**
   * Wraps most components needed to run an instance of the
   * AdministrationService with helper functions.
   */
  class AdministrationServiceTestEnvironment {
    public:

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       */
      AdministrationServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient);

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param serviceLocatorClient The ServiceLocatorClient to use.
       * @param entitlements The entitlement database to use.
       */
      AdministrationServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
        MarketDataService::EntitlementDatabase entitlements);

      ~AdministrationServiceTestEnvironment();

      /**
       * Grants an account administrative privileges.
       * @param account The account to grant administrative privileges to.
       */
      void MakeAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Builds a new AdministrationClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the AdministrationClient.
       */
      std::unique_ptr<VirtualAdministrationClient> MakeClient(
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
            MetaAdministrationServlet<
              Beam::ServiceLocator::ServiceLocatorClientBox,
              LocalAdministrationDataStore*>,
            Beam::ServiceLocator::ServiceLocatorClientBox>,
          ServerConnection*,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder,
          std::shared_ptr<Beam::Threading::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        Beam::ServiceLocator::ServiceLocatorClientBox,
        Beam::Services::MessageProtocol<std::unique_ptr<ClientChannel>,
          Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
          Beam::Codecs::NullEncoder>,
        Beam::Threading::TriggerTimer>;
      Beam::ServiceLocator::ServiceLocatorClientBox m_serviceLocatorClient;
      LocalAdministrationDataStore m_dataStore;
      ServerConnection m_serverConnection;
      ServiceProtocolServletContainer m_container;

      static MarketDataService::EntitlementDatabase MakeDefaultEntitlements(
        Beam::ServiceLocator::ServiceLocatorClientBox& client);
      AdministrationServiceTestEnvironment(
        const AdministrationServiceTestEnvironment&) = delete;
      AdministrationServiceTestEnvironment& operator =(
        const AdministrationServiceTestEnvironment&) = delete;
  };

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient)
    : AdministrationServiceTestEnvironment(std::move(serviceLocatorClient),
        MakeDefaultEntitlements(serviceLocatorClient)) {}

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient,
      MarketDataService::EntitlementDatabase entitlements)
    : m_serviceLocatorClient(std::move(serviceLocatorClient)),
      m_container(Beam::Initialize(m_serviceLocatorClient, Beam::Initialize(
        m_serviceLocatorClient, std::move(entitlements), &m_dataStore)),
        &m_serverConnection,
        boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {}

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    Close();
  }

  inline void AdministrationServiceTestEnvironment::MakeAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto administrators = m_serviceLocatorClient.LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      "administrators");
    m_serviceLocatorClient.Associate(account, administrators);
  }

  inline std::unique_ptr<VirtualAdministrationClient>
      AdministrationServiceTestEnvironment::MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return MakeVirtualAdministrationClient(
      std::make_unique<AdministrationClient<ServiceProtocolClientBuilder>>(
        ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
          std::bind(boost::factory<std::unique_ptr<
            ServiceProtocolClientBuilder::Channel>>(),
            "test_administration_client", std::ref(m_serverConnection)),
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>())));
  }

  inline void AdministrationServiceTestEnvironment::Close() {
    m_container.Close();
  }

  inline MarketDataService::EntitlementDatabase
      AdministrationServiceTestEnvironment::MakeDefaultEntitlements(
      Beam::ServiceLocator::ServiceLocatorClientBox& client) {
    auto entitlementsDirectory = client.MakeDirectory("entitlements",
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    auto globalEntitlementGroup = client.MakeDirectory("global",
      entitlementsDirectory);
    client.Associate(client.GetAccount(), globalEntitlementGroup);
    auto globalEntitlement = MarketDataService::EntitlementDatabase::Entry();
    globalEntitlement.m_name = "global";
    globalEntitlement.m_groupEntry = globalEntitlementGroup;
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
    auto entitlements = MarketDataService::EntitlementDatabase();
    entitlements.Add(globalEntitlement);
    return entitlements;
  }
}

#endif
