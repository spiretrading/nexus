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
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

namespace Nexus::Tests {

  /**
   * Wraps most components needed to run an instance of the Administration
   * service with helper functions.
   */
  class AdministrationServiceTestEnvironment {
    public:

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param client The ServiceLocatorClient to use.
       */
      AdministrationServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox client);

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param client The ServiceLocatorClient to use.
       * @param entitlements The entitlement database to use.
       */
      AdministrationServiceTestEnvironment(
        Beam::ServiceLocator::ServiceLocatorClientBox client,
        MarketDataService::EntitlementDatabase entitlements);

      ~AdministrationServiceTestEnvironment();

      /** Returns this test environment's AdministrationClient. */
      AdministrationClient& get_client();

      /**
       * Grants an account administrative privileges.
       * @param account The account to grant administrative privileges to.
       */
      void make_administrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Returns a new AdministrationClient.
       * @param client The ServiceLocatorClient used to authenticate the
       *        AdministrationClient.
       */
      AdministrationClient
        make_client(Beam::ServiceLocator::ServiceLocatorClientBox client);

      void close();

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
              LocalAdministrationDataStore*,
              Beam::TimeService::LocalTimeClient>,
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
      Beam::ServiceLocator::ServiceLocatorClientBox m_service_locator_client;
      LocalAdministrationDataStore m_data_store;
      ServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;
      boost::optional<AdministrationClient> m_client;

      static MarketDataService::EntitlementDatabase make_default_entitlements(
        Beam::ServiceLocator::ServiceLocatorClientBox& client);
      AdministrationServiceTestEnvironment(
        const AdministrationServiceTestEnvironment&) = delete;
      AdministrationServiceTestEnvironment& operator =(
        const AdministrationServiceTestEnvironment&) = delete;
  };

  /**
   * Uses a ServiceLocatorAccount to make an account suitable for use by a
   * AdministrationServiceTestEnvironment.
   * @param client The ServiceLocatorClient to use, should have root
   *        permissions.
   * @param name The name of the account.
   * @param password The account's password.
   * @return The account's DirectoryEntry.
   */
  template<typename ServiceLocatorClient>
  inline Beam::ServiceLocator::DirectoryEntry make_administrator_account(
      ServiceLocatorClient& client, const std::string& name,
      const std::string& password) {
    auto account = client.MakeAccount(
      name, password, Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    client.StorePermissions(
      account, Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      Beam::ServiceLocator::Permissions().Set(
        Beam::ServiceLocator::Permission::READ).Set(
        Beam::ServiceLocator::Permission::MOVE).Set(
        Beam::ServiceLocator::Permission::ADMINISTRATE));
    return account;
  }

  /**
   * Constructs an AdministrationServiceTestEnvironment using defaults settings
   * from a ServiceLocatorTestEnvironment.
   * @param environment The ServiceLocatorTestEnvironment used to create default
   *        settings.
   */
  inline AdministrationServiceTestEnvironment
      make_administration_service_test_environment(
        Beam::ServiceLocator::Tests::ServiceLocatorTestEnvironment&
          environment) {
    make_administrator_account(
      environment.GetRoot(), "administration_service", "1234");
    return AdministrationServiceTestEnvironment(
      environment.MakeClient("administration_service", "1234"));
  }

  /**
   * Grants all available entitlements to an account.
   * @param environment The test environment that the entitlements are being
   *        granted on.
   * @param account The account to grant the entitlements to.
   */
  inline void grant_all_entitlements(
      AdministrationServiceTestEnvironment& environment,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto entitlements = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& entry :
        environment.get_client().load_entitlements().get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    environment.get_client().store_entitlements(account, entitlements);
  }

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox client)
    : AdministrationServiceTestEnvironment(
        std::move(client), make_default_entitlements(client)) {}

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
      Beam::ServiceLocator::ServiceLocatorClientBox client,
      MarketDataService::EntitlementDatabase entitlements)
      : m_service_locator_client(std::move(client)),
          m_container(Beam::Initialize(m_service_locator_client,
            Beam::Initialize(m_service_locator_client, std::move(entitlements),
              &m_data_store, Beam::Initialize())), &m_server_connection,
            boost::factory<std::shared_ptr<Beam::Threading::TriggerTimer>>()) {
    make_administrator(m_service_locator_client.GetAccount());
    m_client.emplace(make_client(m_service_locator_client));
  }

  inline AdministrationServiceTestEnvironment::
      ~AdministrationServiceTestEnvironment() {
    close();
  }

  inline AdministrationClient&
      AdministrationServiceTestEnvironment::get_client() {
    return *m_client;
  }

  inline void AdministrationServiceTestEnvironment::make_administrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto administrators = m_service_locator_client.LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(),
      "administrators");
    m_service_locator_client.Associate(account, administrators);
  }

  inline AdministrationClient AdministrationServiceTestEnvironment::make_client(
      Beam::ServiceLocator::ServiceLocatorClientBox client) {
    return AdministrationClient(std::in_place_type<
      ServiceAdministrationClient<ServiceProtocolClientBuilder>>,
        ServiceProtocolClientBuilder(
          std::move(client), std::bind_front(boost::factory<std::unique_ptr<
            ServiceProtocolClientBuilder::Channel>>(),
            "test_administration_client", std::ref(m_server_connection)),
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void AdministrationServiceTestEnvironment::close() {
    m_container.Close();
  }

  inline MarketDataService::EntitlementDatabase
      AdministrationServiceTestEnvironment::make_default_entitlements(
        Beam::ServiceLocator::ServiceLocatorClientBox& client) {
    auto entitlements_directory = client.MakeDirectory(
      "entitlements", Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    auto global_entitlement_group =
      client.MakeDirectory("global", entitlements_directory);
    client.Associate(client.GetAccount(), global_entitlement_group);
    auto global_entitlement = MarketDataService::EntitlementDatabase::Entry();
    global_entitlement.m_name = "global";
    global_entitlement.m_group_entry = global_entitlement_group;
    auto venues = std::vector<Venue>();
    for(auto& entry : DEFAULT_VENUES.get_entries()) {
      venues.push_back(entry.m_venue);
    }
    for(auto& venue : venues) {
      global_entitlement.m_applicability[
        MarketDataService::EntitlementKey(venue)].Set(
          MarketDataService::MarketDataType::TIME_AND_SALE);
      global_entitlement.m_applicability[
        MarketDataService::EntitlementKey(venue)].Set(
          MarketDataService::MarketDataType::BOOK_QUOTE);
      global_entitlement.m_applicability[
        MarketDataService::EntitlementKey(venue)].Set(
          MarketDataService::MarketDataType::BBO_QUOTE);
      global_entitlement.m_applicability[
        MarketDataService::EntitlementKey(venue)].Set(
          MarketDataService::MarketDataType::ORDER_IMBALANCE);
    }
    auto entitlements = MarketDataService::EntitlementDatabase();
    entitlements.add(global_entitlement);
    return entitlements;
  }
}

#endif
