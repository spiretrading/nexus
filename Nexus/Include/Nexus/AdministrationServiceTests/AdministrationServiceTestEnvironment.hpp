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
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
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
      explicit AdministrationServiceTestEnvironment(
        Beam::ServiceLocatorClient client);

      /**
       * Constructs an AdministrationServiceTestEnvironment.
       * @param client The ServiceLocatorClient to use.
       * @param entitlements The entitlement database to use.
       */
      AdministrationServiceTestEnvironment(
        Beam::ServiceLocatorClient client, EntitlementDatabase entitlements);

      ~AdministrationServiceTestEnvironment();

      /** Returns this test environment's AdministrationClient. */
      AdministrationClient& get_client();

      /**
       * Grants an account administrative privileges.
       * @param account The account to grant administrative privileges to.
       */
      void make_administrator(const Beam::DirectoryEntry& account);

      /**
       * Returns a new AdministrationClient.
       * @param client The ServiceLocatorClient used to authenticate the
       *        AdministrationClient.
       */
      AdministrationClient make_client(
        Beam::Ref<Beam::ServiceLocatorClient> client);

      void close();

    private:
      using ServiceProtocolServletContainer =
        Beam::ServiceProtocolServletContainer<
          Beam::MetaAuthenticationServletAdapter<MetaAdministrationServlet<
            Beam::ServiceLocatorClient, LocalAdministrationDataStore*,
            Beam::LocalTimeClient>, Beam::ServiceLocatorClient>,
          Beam::LocalServerConnection*, Beam::BinarySender<Beam::SharedBuffer>,
          Beam::NullEncoder, std::shared_ptr<Beam::TriggerTimer>>;
      using ServiceProtocolClientBuilder =
        Beam::AuthenticatedServiceProtocolClientBuilder<
          Beam::ServiceLocatorClient,
          Beam::MessageProtocol<std::unique_ptr<Beam::LocalClientChannel>,
            Beam::BinarySender<Beam::SharedBuffer>, Beam::NullEncoder>,
          Beam::TriggerTimer>;
      Beam::ServiceLocatorClient m_service_locator_client;
      LocalAdministrationDataStore m_data_store;
      Beam::LocalServerConnection m_server_connection;
      ServiceProtocolServletContainer m_container;
      boost::optional<AdministrationClient> m_client;

      static EntitlementDatabase make_default_entitlements(
        Beam::ServiceLocatorClient& client);
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
  inline Beam::DirectoryEntry make_administrator_account(
      Beam::IsServiceLocatorClient auto& client, const std::string& name,
      const std::string& password) {
    auto account =
      client.make_account(name, password, Beam::DirectoryEntry::STAR_DIRECTORY);
    client.store(account, Beam::DirectoryEntry::STAR_DIRECTORY,
      Beam::Permissions().
        set(Beam::Permission::READ).
        set(Beam::Permission::MOVE).
        set(Beam::Permission::ADMINISTRATE));
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
        Beam::Tests::ServiceLocatorTestEnvironment& environment) {
    make_administrator_account(
      environment.get_root(), "administration_service", "1234");
    return AdministrationServiceTestEnvironment(
      environment.make_client("administration_service", "1234"));
  }

  /**
   * Grants all available entitlements to an account.
   * @param environment The test environment that the entitlements are being
   *        granted on.
   * @param account The account to grant the entitlements to.
   */
  inline void grant_all_entitlements(
      AdministrationServiceTestEnvironment& environment,
      const Beam::DirectoryEntry& account) {
    auto entitlements = std::vector<Beam::DirectoryEntry>();
    for(auto& entry :
        environment.get_client().load_entitlements().get_entries()) {
      entitlements.push_back(entry.m_group_entry);
    }
    environment.get_client().store_entitlements(account, entitlements);
  }

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(Beam::ServiceLocatorClient client)
    : AdministrationServiceTestEnvironment(
        std::move(client), make_default_entitlements(client)) {}

  inline AdministrationServiceTestEnvironment::
    AdministrationServiceTestEnvironment(
      Beam::ServiceLocatorClient client, EntitlementDatabase entitlements)
      : m_service_locator_client(std::move(client)),
          m_container(Beam::init(m_service_locator_client,
            Beam::init(m_service_locator_client, std::move(entitlements),
              &m_data_store, Beam::init())), &m_server_connection,
            boost::factory<std::shared_ptr<Beam::TriggerTimer>>()) {
    make_administrator(m_service_locator_client.get_account());
    m_client.emplace(make_client(Beam::Ref(m_service_locator_client)));
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
      const Beam::DirectoryEntry& account) {
    auto administrators = m_service_locator_client.load_directory_entry(
      Beam::DirectoryEntry::STAR_DIRECTORY, "administrators");
    m_service_locator_client.associate(account, administrators);
  }

  inline AdministrationClient AdministrationServiceTestEnvironment::make_client(
      Beam::Ref<Beam::ServiceLocatorClient> client) {
    return AdministrationClient(std::in_place_type<
      ServiceAdministrationClient<ServiceProtocolClientBuilder>>,
        ServiceProtocolClientBuilder(
          Beam::Ref(client), std::bind_front(boost::factory<std::unique_ptr<
            ServiceProtocolClientBuilder::Channel>>(),
            "test_administration_client", std::ref(m_server_connection)),
          boost::factory<
            std::unique_ptr<ServiceProtocolClientBuilder::Timer>>()));
  }

  inline void AdministrationServiceTestEnvironment::close() {
    m_container.close();
  }

  inline EntitlementDatabase
      AdministrationServiceTestEnvironment::make_default_entitlements(
        Beam::ServiceLocatorClient& client) {
    auto entitlements_directory = client.make_directory(
      "entitlements", Beam::DirectoryEntry::STAR_DIRECTORY);
    auto global_entitlement_group =
      client.make_directory("global", entitlements_directory);
    client.associate(client.get_account(), global_entitlement_group);
    auto global_entitlement = EntitlementDatabase::Entry();
    global_entitlement.m_name = "global";
    global_entitlement.m_group_entry = global_entitlement_group;
    auto venues = std::vector<Venue>();
    for(auto& entry : DEFAULT_VENUES.get_entries()) {
      venues.push_back(entry.m_venue);
    }
    for(auto& venue : venues) {
      global_entitlement.m_applicability[EntitlementKey(venue)].set(
        MarketDataType::TIME_AND_SALE);
      global_entitlement.m_applicability[EntitlementKey(venue)].set(
        MarketDataType::BOOK_QUOTE);
      global_entitlement.m_applicability[EntitlementKey(venue)].set(
        MarketDataType::BBO_QUOTE);
      global_entitlement.m_applicability[EntitlementKey(venue)].set(
        MarketDataType::ORDER_IMBALANCE);
    }
    auto entitlements = EntitlementDatabase();
    entitlements.add(global_entitlement);
    return entitlements;
  }
}

#endif
