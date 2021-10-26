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
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
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

      /** Returns this test environment's AdministrationClient. */
      AdministrationClientBox& GetClient();

      /**
       * Grants an account administrative privileges.
       * @param account The account to grant administrative privileges to.
       */
      void MakeAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Returns a new AdministrationClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the AdministrationClient.
       */
      AdministrationClientBox MakeClient(
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
      boost::optional<AdministrationClientBox> m_client;

      static MarketDataService::EntitlementDatabase MakeDefaultEntitlements(
        Beam::ServiceLocator::ServiceLocatorClientBox& client);
      AdministrationServiceTestEnvironment(
        const AdministrationServiceTestEnvironment&) = delete;
      AdministrationServiceTestEnvironment& operator =(
        const AdministrationServiceTestEnvironment&) = delete;
  };

  /**
   * Uses a ServiceLocatorAccount to make an account suitable for use by a
   * AdministrationServiceTestEnvironment.
   * @param serviceLocatorClient The ServiceLocatorClient to use, should have
   *        root permissions.
   * @param name The name of the account.
   * @param password The account's password.
   * @return The account's DirectoryEntry.
   */
  template<typename ServiceLocatorClient>
  inline Beam::ServiceLocator::DirectoryEntry MakeAdministratorAccount(
      ServiceLocatorClient& serviceLocatorClient, const std::string& name,
      const std::string& password) {
    auto account = serviceLocatorClient.MakeAccount(name, password,
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    serviceLocatorClient.StorePermissions(account,
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      Beam::ServiceLocator::Permissions().Set(
        Beam::ServiceLocator::Permission::READ).Set(
        Beam::ServiceLocator::Permission::MOVE).Set(
        Beam::ServiceLocator::Permission::ADMINISTRATE));
    return account;
  }

  /**
   * Constructs an AdministrationServiceTestEnvironment using defaults settings
   * from a ServiceLocatorTestEnvironment.
   * @param serviceLocatorEnvironment The ServiceLocatorTestEnvironment used
   *        to create default settings.
   */
  inline AdministrationServiceTestEnvironment
      MakeAdministrationServiceTestEnvironment(
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
          serviceLocatorEnvironment) {
    MakeAdministratorAccount(serviceLocatorEnvironment.GetRoot(),
      "administration_service", "1234");
    return AdministrationServiceTestEnvironment(
      serviceLocatorEnvironment.MakeClient("administration_service", "1234"));
  }

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
            boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {
    MakeAdministrator(m_serviceLocatorClient.GetAccount());
    m_client.emplace(MakeClient(m_serviceLocatorClient));
  }

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    Close();
  }

  inline AdministrationClientBox&
      AdministrationServiceTestEnvironment::GetClient() {
    return *m_client;
  }

  inline void AdministrationServiceTestEnvironment::MakeAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto administrators = m_serviceLocatorClient.LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      "administrators");
    m_serviceLocatorClient.Associate(account, administrators);
  }

  inline AdministrationClientBox
      AdministrationServiceTestEnvironment::MakeClient(
        Beam::ServiceLocator::ServiceLocatorClientBox serviceLocatorClient) {
    return AdministrationClientBox(std::in_place_type<
      AdministrationClient<ServiceProtocolClientBuilder>>,
        ServiceProtocolClientBuilder(std::move(serviceLocatorClient),
          std::bind_front(boost::factory<std::unique_ptr<
            ServiceProtocolClientBuilder::Channel>>(),
            "test_administration_client", std::ref(m_serverConnection)),
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
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
