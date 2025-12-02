#ifndef NEXUS_ADMINISTRATION_CLIENT_HPP
#define NEXUS_ADMINISTRATION_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Queues/Publisher.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {

  /** Used to push updates about an account's RiskState. */
  using RiskStateQueue = Beam::AbstractQueue<RiskState>;

  /** Used to push updates about an account's RiskParameters. */
  using RiskParametersQueue = Beam::AbstractQueue<RiskParameters>;

  /** Checks if a type implements an AdministrationClient. */
  template<typename T>
  concept IsAdministrationClient =
    Beam::IsConnection<T> && requires(T& client) {
      { client.load_accounts_by_roles(std::declval<AccountRoles>()) } ->
          std::same_as<std::vector<Beam::DirectoryEntry>>;
      { client.load_administrators_root_entry() } ->
          std::same_as<Beam::DirectoryEntry>;
      { client.load_services_root_entry() } ->
          std::same_as<Beam::DirectoryEntry>;
      { client.load_trading_groups_root_entry() } ->
          std::same_as<Beam::DirectoryEntry>;
      { client.check_administrator(
          std::declval<const Beam::DirectoryEntry&>()) } -> std::same_as<bool>;
      { client.load_account_roles(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<AccountRoles>;
      { client.load_account_roles(std::declval<const Beam::DirectoryEntry&>(),
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<AccountRoles>;
      { client.load_parent_trading_group(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<Beam::DirectoryEntry>;
      { client.load_identity(std::declval<const Beam::DirectoryEntry&>()) } ->
          std::same_as<AccountIdentity>;
      client.store(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const AccountIdentity&>());
      { client.load_trading_group(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<TradingGroup>;
      { client.load_managed_trading_groups(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<std::vector<Beam::DirectoryEntry>>;
      { client.load_administrators() } ->
          std::same_as<std::vector<Beam::DirectoryEntry>>;
      { client.load_services() } ->
          std::same_as<std::vector<Beam::DirectoryEntry>>;
      { client.load_entitlements() } -> std::same_as<EntitlementDatabase>;
      { client.load_entitlements(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<std::vector<Beam::DirectoryEntry>>;
      client.store_entitlements(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<Beam::DirectoryEntry>&>());
      { client.get_risk_parameters_publisher(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<const Beam::Publisher<RiskParameters>&>;
      client.store(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const RiskParameters&>());
      { client.get_risk_state_publisher(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<const Beam::Publisher<RiskState>&>;
      client.store(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const RiskState&>());
      { client.load_account_modification_request(
          std::declval<AccountModificationRequest::Id>()) } ->
            std::same_as<AccountModificationRequest>;
      { client.load_account_modification_request_ids(
          std::declval<const Beam::DirectoryEntry&>(),
          std::declval<AccountModificationRequest::Id>(),
          std::declval<int>()) } ->
            std::same_as<std::vector<AccountModificationRequest::Id>>;
      { client.load_managed_account_modification_request_ids(
          std::declval<const Beam::DirectoryEntry&>(),
          std::declval<AccountModificationRequest::Id>(),
          std::declval<int>()) } ->
            std::same_as<std::vector<AccountModificationRequest::Id>>;
      { client.load_entitlement_modification(
          std::declval<AccountModificationRequest::Id>()) } ->
            std::same_as<EntitlementModification>;
      { client.submit(std::declval<const Beam::DirectoryEntry&>(),
          std::declval<const EntitlementModification&>(),
          std::declval<const Message&>()) } ->
            std::same_as<AccountModificationRequest>;
      { client.load_risk_modification(
          std::declval<AccountModificationRequest::Id>()) } ->
            std::same_as<RiskModification>;
      { client.submit(std::declval<const Beam::DirectoryEntry&>(),
          std::declval<const RiskModification&>(),
          std::declval<const Message&>()) } ->
            std::same_as<AccountModificationRequest>;
      { client.load_account_modification_request_status(
          std::declval<AccountModificationRequest::Id>()) } ->
            std::same_as<AccountModificationRequest::Update>;
      { client.approve_account_modification_request(
          std::declval<AccountModificationRequest::Id>(),
          std::declval<const Message&>()) } ->
            std::same_as<AccountModificationRequest::Update>;
      { client.reject_account_modification_request(
          std::declval<AccountModificationRequest::Id>(),
          std::declval<const Message&>()) } ->
            std::same_as<AccountModificationRequest::Update>;
      { client.load_message(std::declval<Message::Id>()) } ->
          std::same_as<Message>;
      { client.load_message_ids(
          std::declval<AccountModificationRequest::Id>()) } ->
            std::same_as<std::vector<Message::Id>>;
      { client.send_account_modification_request_message(
          std::declval<AccountModificationRequest::Id>(),
          std::declval<const Message&>()) } -> std::same_as<Message>;
    };

  /** Provides a generic interface over an arbitrary AdministrationClient. */
  class AdministrationClient {
    public:

      /**
       * Constructs an AdministrationClient of a specified type using
       * emplacement.
       * @tparam T The type of administration client to emplace.
       * @param args The arguments to pass to the emplaced administration
       *        client.
       */
      template<IsAdministrationClient T, typename... Args>
      explicit AdministrationClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an AdministrationClient by referencing an existing client.
       * @param client The client to reference.
       */
      template<Beam::DisableCopy<AdministrationClient> T> requires
        IsAdministrationClient<Beam::dereference_t<T>>
      AdministrationClient(T&& client);

      AdministrationClient(const AdministrationClient&) = default;
      AdministrationClient(AdministrationClient&&) = default;

      /**
       * Loads the list of accounts that match a set of roles.
       * @param roles The roles to match.
       * @return The list of directory entries of accounts that match the
       *         specified roles.
       */
      std::vector<Beam::DirectoryEntry> load_accounts_by_roles(
        AccountRoles roles);

      /** Loads the DirectoryEntry containing all administrators. */
      Beam::DirectoryEntry load_administrators_root_entry();

      /** Loads the DirectoryEntry containing all service accounts. */
      Beam::DirectoryEntry load_services_root_entry();

      /** Loads the DirectoryEntry containing all trading groups. */
      Beam::DirectoryEntry load_trading_groups_root_entry();

      /**
       * Returns <code>true</code> iff an account is an administrator.
       * @param account The account to test.
       * @return <code>true</code> iff the <i>account</i> is an administrator.
       */
      bool check_administrator(const Beam::DirectoryEntry& account);

      /**
       * Returns an accounts roles.
       * @param account The account to lookup.
       * @return The roles associated with the <i>account</i>.
       */
      AccountRoles load_account_roles(const Beam::DirectoryEntry& account);

      /**
       * Returns the roles one account has over another.
       * @param parent The account whose roles are to be loaded.
       * @param child The account being supervised.
       * @return The roles that the <i>parent</i> account has over the
       *         <i>child</i> account.
       */
      AccountRoles load_account_roles(
        const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child);

      /**
       * Loads the DirectoryEntry representing an account's trading group.
       * @param account The account whose trading group is to be loaded.
       * @return The directory of the <i>account</i>'s trading group.
       */
      Beam::DirectoryEntry load_parent_trading_group(
        const Beam::DirectoryEntry& account);

      /**
       * Loads an account's identity.
       * @param account The account whose identity is to be loaded.
       * @return The AccountIdentity for the specified <i>account</i>.
       */
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);

      /**
       * Sets the identity for an account.
       * @param account The account to set the identity for.
       * @param identity The AccountIdentity to assign to the <i>account</i>.
       */
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);

      /**
       * Loads a TradingGroup from its DirectoryEntry.
       * @param directory The DirectoryEntry of the TradingGroup to load.
       * @return The TradingGroup represented by the specified <i>directory</i>.
       */
      TradingGroup load_trading_group(const Beam::DirectoryEntry& directory);

      /**
       * Loads the DirectoryEntries of TradingGroups managed by an account.
       * @param account The account to load the TradingGroups from.
       * @return The list of TradingGroups managed by the <i>account</i>.
       */
      std::vector<Beam::DirectoryEntry> load_managed_trading_groups(
        const Beam::DirectoryEntry& account);

      /**
       * Loads the system administrators.
       * @return The list of system administrators.
       */
      std::vector<Beam::DirectoryEntry> load_administrators();

      /**
       * Loads the accounts providing system services.
       * @return The list of accounts providing system services.
       */
      std::vector<Beam::DirectoryEntry> load_services();

      /**
       * Loads the EntitlementDatabase.
       * @return The EntitlementDatabase.
       */
      EntitlementDatabase load_entitlements();

      /**
       * Loads the entitlements granted to an account.
       * @param account The account to load the entitlements for.
       * @return The list of entitlements granted to the <i>account</i>.
       */
      std::vector<Beam::DirectoryEntry> load_entitlements(
        const Beam::DirectoryEntry& account);

      /**
       * Sets an account's entitlements.
       * @param account The account of the entitlements to set.
       * @param entitlements The list of entitlements to grant to the
       *        <i>account</i>.
       */
      void store_entitlements(const Beam::DirectoryEntry& account,
        const std::vector<Beam::DirectoryEntry>& entitlements);

      /**
       * Returns the object publishing an account's RiskParameters.
       * @param account The account to monitor.
       */
      const Beam::Publisher<RiskParameters>& get_risk_parameters_publisher(
        const Beam::DirectoryEntry& account);

      /**
       * Sets an account's RiskParameters.
       * @param account The account whose RiskParameters are to be set.
       * @param parameters The RiskParameters to assign to the
       *        <i>account</i>.
       */
      void store(
        const Beam::DirectoryEntry& account, const RiskParameters& parameters);

      /**
       * Returns the object publishing an account's RiskState.
       * @param account The account to monitor.
       */
      const Beam::Publisher<RiskState>& get_risk_state_publisher(
        const Beam::DirectoryEntry& account);

      /**
       * Sets an account's RiskState.
       * @param account The account to set RiskState of.
       * @param state The <i>account</i>'s current RiskState.
       */
      void store(const Beam::DirectoryEntry& account, const RiskState& state);

      /**
       * Loads an account modification request.
       * @param id The id of the request to load.
       * @return The request with the specified <i>id</i>.
       */
      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);

      /**
       * Given an account, loads the ids of requests to modify that account.
       * @param account The account whose requests are to be loaded.
       * @param start_id The id of the first request to load (exclusive) or -1
       *        to start with the most recent request.
       * @param max_count The maximum number of ids to load.
       * @return The list of account modification requests.
       */
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);

      /**
       * Given an account, loads the ids of requests that the account is
       * authorized to manage.
       * @param account The account managing modifications.
       * @param start_id The id of the first request to load (exclusive) or -1
       *        to start with the most recent request.
       * @param max_count The maximum number of ids to load.
       * @return The list of account modification requests.
       */
      std::vector<AccountModificationRequest::Id>
        load_managed_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);

      /**
       * Loads an entitlement modification.
       * @param id The id of the request to load.
       * @return The entitlement modification with the specified <i>id</i>.
       */
      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);

      /**
       * Submits a request to modify an account's entitlements.
       * @param account The account to modify.
       * @param modification The modification to apply.
       * @param comment The comment to associate with the request.
       * @return An object representing the request.
       */
      AccountModificationRequest submit(const Beam::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);

      /**
       * Loads a risk modification.
       * @param id The id of the request to load.
       * @return The risk modification with the specified <i>id</i>.
       */
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);

      /**
       * Submits a request to modify an account's risk parameters.
       * @param account The account to modify.
       * @param modification The modification to apply.
       * @param comment The comment to associate with the request.
       * @return An object representing the request.
       */
      AccountModificationRequest submit(const Beam::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment);

      /**
       * Loads the status of an account modification request.
       * @param id The id of the request.
       * @return The update representing the current status of the request.
       */
      AccountModificationRequest::Update
        load_account_modification_request_status(
          AccountModificationRequest::Id id);

      /**
       * Approves an account modification request.
       * @param id The id of the request to approve.
       * @param comment The comment to associate with the update.
       * @return An object representing the update.
       */
      AccountModificationRequest::Update approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment);

      /**
       * Rejects an account modification request.
       * @param id The id of the request to reject.
       * @param comment The comment to associate with the update.
       * @return An object representing the update.
       */
      AccountModificationRequest::Update reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment);

      /**
       * Loads a message.
       * @param id The id of the message.
       * @return The message with the specified <i>id</i>.
       */
      Message load_message(Message::Id id);

      /**
       * Loads the list of messages associated with an account modification.
       * @param id The id of the request.
       * @return A list of message ids associated with the request.
       */
      std::vector<Message::Id> load_message_ids(
        AccountModificationRequest::Id id);

      /**
       * Appends a message to an account modification request.
       * @param id The id of the request to send the message to.
       * @param message The message to append.
       * @return The appended message.
       */
      Message send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message);

      void close();

    private:
      struct VirtualAdministrationClient {
        virtual ~VirtualAdministrationClient() = default;

        virtual std::vector<Beam::DirectoryEntry> load_accounts_by_roles(
          AccountRoles roles) = 0;
        virtual Beam::DirectoryEntry load_administrators_root_entry() = 0;
        virtual Beam::DirectoryEntry load_services_root_entry() = 0;
        virtual Beam::DirectoryEntry load_trading_groups_root_entry() = 0;
        virtual bool check_administrator(
          const Beam::DirectoryEntry& account) = 0;
        virtual AccountRoles load_account_roles(
          const Beam::DirectoryEntry& account) = 0;
        virtual AccountRoles load_account_roles(
          const Beam::DirectoryEntry& parent,
          const Beam::DirectoryEntry& child) = 0;
        virtual Beam::DirectoryEntry load_parent_trading_group(
          const Beam::DirectoryEntry& account) = 0;
        virtual AccountIdentity load_identity(
          const Beam::DirectoryEntry& account) = 0;
        virtual void store(const Beam::DirectoryEntry& account,
          const AccountIdentity& identity) = 0;
        virtual TradingGroup load_trading_group(
          const Beam::DirectoryEntry& directory) = 0;
        virtual std::vector<Beam::DirectoryEntry> load_managed_trading_groups(
          const Beam::DirectoryEntry& account) = 0;
        virtual std::vector<Beam::DirectoryEntry> load_administrators() = 0;
        virtual std::vector<Beam::DirectoryEntry> load_services() = 0;
        virtual EntitlementDatabase load_entitlements() = 0;
        virtual std::vector<Beam::DirectoryEntry> load_entitlements(
          const Beam::DirectoryEntry& account) = 0;
        virtual void store_entitlements(const Beam::DirectoryEntry& account,
          const std::vector<Beam::DirectoryEntry>& entitlements) = 0;
        virtual const Beam::Publisher<RiskParameters>&
          get_risk_parameters_publisher(
            const Beam::DirectoryEntry& account) = 0;
        virtual void store(const Beam::DirectoryEntry& account,
          const RiskParameters& risk_parameters) = 0;
        virtual const Beam::Publisher<RiskState>& get_risk_state_publisher(
          const Beam::DirectoryEntry& account) = 0;
        virtual void store(const Beam::DirectoryEntry& account,
          const RiskState& risk_state) = 0;
        virtual AccountModificationRequest load_account_modification_request(
          AccountModificationRequest::Id id) = 0;
        virtual std::vector<AccountModificationRequest::Id>
          load_account_modification_request_ids(
            const Beam::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) = 0;
        virtual std::vector<AccountModificationRequest::Id>
          load_managed_account_modification_request_ids(
            const Beam::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) = 0;
        virtual EntitlementModification load_entitlement_modification(
          AccountModificationRequest::Id id) = 0;
        virtual AccountModificationRequest submit(
          const Beam::DirectoryEntry& account,
          const EntitlementModification& modification,
          const Message& comment) = 0;
        virtual RiskModification load_risk_modification(
          AccountModificationRequest::Id id) = 0;
        virtual AccountModificationRequest submit(
          const Beam::DirectoryEntry& account,
          const RiskModification& modification, const Message& comment) = 0;
        virtual AccountModificationRequest::Update
          load_account_modification_request_status(
            AccountModificationRequest::Id id) = 0;
        virtual AccountModificationRequest::Update
          approve_account_modification_request(
            AccountModificationRequest::Id id, const Message& comment) = 0;
        virtual AccountModificationRequest::Update
          reject_account_modification_request(
            AccountModificationRequest::Id id, const Message& comment) = 0;
        virtual Message load_message(Message::Id id) = 0;
        virtual std::vector<Message::Id> load_message_ids(
          AccountModificationRequest::Id id) = 0;
        virtual Message send_account_modification_request_message(
          AccountModificationRequest::Id id, const Message& message) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedAdministrationClient final : VirtualAdministrationClient {
        using AdministrationClient = C;
        Beam::local_ptr_t<AdministrationClient> m_client;

        template<typename... Args>
        WrappedAdministrationClient(Args&&... args);

        std::vector<Beam::DirectoryEntry> load_accounts_by_roles(
          AccountRoles roles) override;
        Beam::DirectoryEntry load_administrators_root_entry() override;
        Beam::DirectoryEntry load_services_root_entry() override;
        Beam::DirectoryEntry load_trading_groups_root_entry() override;
        bool check_administrator(
          const Beam::DirectoryEntry& account) override;
        AccountRoles load_account_roles(
          const Beam::DirectoryEntry& account) override;
        AccountRoles load_account_roles(const Beam::DirectoryEntry& parent,
          const Beam::DirectoryEntry& child) override;
        Beam::DirectoryEntry load_parent_trading_group(
          const Beam::DirectoryEntry& account) override;
        AccountIdentity load_identity(
          const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const AccountIdentity& identity) override;
        TradingGroup load_trading_group(
          const Beam::DirectoryEntry& directory) override;
        std::vector<Beam::DirectoryEntry> load_managed_trading_groups(
          const Beam::DirectoryEntry& account) override;
        std::vector<Beam::DirectoryEntry> load_administrators() override;
        std::vector<Beam::DirectoryEntry> load_services() override;
        EntitlementDatabase load_entitlements() override;
        std::vector<Beam::DirectoryEntry> load_entitlements(
          const Beam::DirectoryEntry& account) override;
        void store_entitlements(const Beam::DirectoryEntry& account,
          const std::vector<Beam::DirectoryEntry>& entitlements) override;
        const Beam::Publisher<RiskParameters>& get_risk_parameters_publisher(
          const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const RiskParameters& risk_parameters) override;
        const Beam::Publisher<RiskState>& get_risk_state_publisher(
          const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const RiskState& risk_state) override;
        AccountModificationRequest load_account_modification_request(
          AccountModificationRequest::Id id) override;
        std::vector<AccountModificationRequest::Id>
          load_account_modification_request_ids(
            const Beam::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) override;
        std::vector<AccountModificationRequest::Id>
          load_managed_account_modification_request_ids(
            const Beam::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) override;
        EntitlementModification load_entitlement_modification(
          AccountModificationRequest::Id id) override;
        AccountModificationRequest submit(const Beam::DirectoryEntry& account,
          const EntitlementModification& modification,
          const Message& comment) override;
        RiskModification load_risk_modification(
          AccountModificationRequest::Id id) override;
        AccountModificationRequest submit(const Beam::DirectoryEntry& account,
          const RiskModification& modification,
          const Message& comment) override;
        AccountModificationRequest::Update
          load_account_modification_request_status(
            AccountModificationRequest::Id id) override;
        AccountModificationRequest::Update approve_account_modification_request(
          AccountModificationRequest::Id id, const Message& comment) override;
        AccountModificationRequest::Update reject_account_modification_request(
          AccountModificationRequest::Id id, const Message& comment) override;
        Message load_message(Message::Id id) override;
        std::vector<Message::Id> load_message_ids(
          AccountModificationRequest::Id id) override;
        Message send_account_modification_request_message(
          AccountModificationRequest::Id id, const Message& message) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualAdministrationClient> m_client;
  };

  /**
   * Loads an account's RiskParameters.
   * @param client The ServiceAdministrationClient used to load the parameters.
   * @param account The account whose parameters are to be loaded.
   * @return The <i>account</i>'s RiskParameters.
   */
  RiskParameters load_risk_parameters(IsAdministrationClient auto& client,
      const Beam::DirectoryEntry& account) {
    auto queue = std::make_shared<Beam::StateQueue<RiskParameters>>();
    client.get_risk_parameters_publisher(account).monitor(queue);
    return queue->pop();
  }

  template<IsAdministrationClient T, typename... Args>
  AdministrationClient::AdministrationClient(
    std::in_place_type_t<T>, Args&&... args)
    : m_client(Beam::make_virtual_ptr<WrappedAdministrationClient<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<AdministrationClient> T> requires
    IsAdministrationClient<Beam::dereference_t<T>>
  AdministrationClient::AdministrationClient(T&& client)
    : m_client(Beam::make_virtual_ptr<WrappedAdministrationClient<
        std::remove_cvref_t<T>>>(std::forward<T>(client))) {}

  inline std::vector<Beam::DirectoryEntry>
      AdministrationClient::load_accounts_by_roles(AccountRoles roles) {
    return m_client->load_accounts_by_roles(roles);
  }

  inline Beam::DirectoryEntry
      AdministrationClient::load_administrators_root_entry() {
    return m_client->load_administrators_root_entry();
  }

  inline Beam::DirectoryEntry
      AdministrationClient::load_services_root_entry() {
    return m_client->load_services_root_entry();
  }

  inline Beam::DirectoryEntry
      AdministrationClient::load_trading_groups_root_entry() {
    return m_client->load_trading_groups_root_entry();
  }

  inline bool AdministrationClient::check_administrator(
      const Beam::DirectoryEntry& account) {
    return m_client->check_administrator(account);
  }

  inline AccountRoles AdministrationClient::load_account_roles(
      const Beam::DirectoryEntry& account) {
    return m_client->load_account_roles(account);
  }

  inline AccountRoles AdministrationClient::load_account_roles(
      const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child) {
    return m_client->load_account_roles(parent, child);
  }

  inline Beam::DirectoryEntry AdministrationClient::load_parent_trading_group(
      const Beam::DirectoryEntry& account) {
    return m_client->load_parent_trading_group(account);
  }

  inline AccountIdentity AdministrationClient::load_identity(
      const Beam::DirectoryEntry& account) {
    return m_client->load_identity(account);
  }

  inline void AdministrationClient::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    m_client->store(account, identity);
  }

  inline TradingGroup AdministrationClient::load_trading_group(
      const Beam::DirectoryEntry& directory) {
    return m_client->load_trading_group(directory);
  }

  inline std::vector<Beam::DirectoryEntry>
      AdministrationClient::load_managed_trading_groups(
        const Beam::DirectoryEntry& account) {
    return m_client->load_managed_trading_groups(account);
  }

  inline std::vector<Beam::DirectoryEntry>
      AdministrationClient::load_administrators() {
    return m_client->load_administrators();
  }

  inline std::vector<Beam::DirectoryEntry>
      AdministrationClient::load_services() {
    return m_client->load_services();
  }

  inline EntitlementDatabase AdministrationClient::load_entitlements() {
    return m_client->load_entitlements();
  }

  inline std::vector<Beam::DirectoryEntry>
      AdministrationClient::load_entitlements(
        const Beam::DirectoryEntry& account) {
    return m_client->load_entitlements(account);
  }

  inline void AdministrationClient::store_entitlements(
      const Beam::DirectoryEntry& account,
      const std::vector<Beam::DirectoryEntry>& entitlements) {
    m_client->store_entitlements(account, entitlements);
  }

  inline const Beam::Publisher<RiskParameters>&
      AdministrationClient::get_risk_parameters_publisher(
        const Beam::DirectoryEntry& account) {
    return m_client->get_risk_parameters_publisher(account);
  }

  inline void AdministrationClient::store(
      const Beam::DirectoryEntry& account, const RiskParameters& parameters) {
    m_client->store(account, parameters);
  }

  inline const Beam::Publisher<RiskState>&
      AdministrationClient::get_risk_state_publisher(
        const Beam::DirectoryEntry& account) {
    return m_client->get_risk_state_publisher(account);
  }

  inline void AdministrationClient::store(
      const Beam::DirectoryEntry& account, const RiskState& state) {
    m_client->store(account, state);
  }

  inline AccountModificationRequest AdministrationClient::
      load_account_modification_request(AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request(id);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationClient::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationClient::load_managed_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_managed_account_modification_request_ids(
      account, start_id, max_count);
  }

  inline EntitlementModification AdministrationClient::
      load_entitlement_modification(AccountModificationRequest::Id id) {
    return m_client->load_entitlement_modification(id);
  }

  inline AccountModificationRequest AdministrationClient::submit(
      const Beam::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    return m_client->submit(account, modification, comment);
  }

  inline RiskModification AdministrationClient::load_risk_modification(
      AccountModificationRequest::Id id) {
    return m_client->load_risk_modification(id);
  }

  inline AccountModificationRequest AdministrationClient::submit(
      const Beam::DirectoryEntry& account, const RiskModification& modification,
      const Message& comment) {
    return m_client->submit(account, modification, comment);
  }

  inline AccountModificationRequest::Update
      AdministrationClient::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request_status(id);
  }

  inline AccountModificationRequest::Update
      AdministrationClient::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->approve_account_modification_request(id, comment);
  }

  inline AccountModificationRequest::Update
      AdministrationClient::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->reject_account_modification_request(id, comment);
  }

  inline Message AdministrationClient::load_message(Message::Id id) {
    return m_client->load_message(id);
  }

  inline std::vector<Message::Id> AdministrationClient::load_message_ids(
      AccountModificationRequest::Id id) {
    return m_client->load_message_ids(id);
  }

  inline Message AdministrationClient::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return m_client->send_account_modification_request_message(id, message);
  }

  inline void AdministrationClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  AdministrationClient::WrappedAdministrationClient<C>::
    WrappedAdministrationClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::vector<Beam::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_accounts_by_roles(AccountRoles roles) {
    return m_client->load_accounts_by_roles(roles);
  }

  template<typename C>
  Beam::DirectoryEntry AdministrationClient::WrappedAdministrationClient<C>::
      load_administrators_root_entry() {
    return m_client->load_administrators_root_entry();
  }

  template<typename C>
  Beam::DirectoryEntry AdministrationClient::WrappedAdministrationClient<C>::
      load_services_root_entry() {
    return m_client->load_services_root_entry();
  }

  template<typename C>
  Beam::DirectoryEntry AdministrationClient::WrappedAdministrationClient<C>::
      load_trading_groups_root_entry() {
    return m_client->load_trading_groups_root_entry();
  }

  template<typename C>
  bool AdministrationClient::WrappedAdministrationClient<C>::
      check_administrator(const Beam::DirectoryEntry& account) {
    return m_client->check_administrator(account);
  }

  template<typename C>
  AccountRoles AdministrationClient::WrappedAdministrationClient<C>::
      load_account_roles(const Beam::DirectoryEntry& account) {
    return m_client->load_account_roles(account);
  }

  template<typename C>
  AccountRoles AdministrationClient::WrappedAdministrationClient<C>::
      load_account_roles(const Beam::DirectoryEntry& parent,
        const Beam::DirectoryEntry& child) {
    return m_client->load_account_roles(parent, child);
  }

  template<typename C>
  Beam::DirectoryEntry AdministrationClient::WrappedAdministrationClient<C>::
      load_parent_trading_group(const Beam::DirectoryEntry& account) {
    return m_client->load_parent_trading_group(account);
  }

  template<typename C>
  AccountIdentity AdministrationClient::WrappedAdministrationClient<C>::
      load_identity(const Beam::DirectoryEntry& account) {
    return m_client->load_identity(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    m_client->store(account, identity);
  }

  template<typename C>
  TradingGroup AdministrationClient::WrappedAdministrationClient<C>::
      load_trading_group(const Beam::DirectoryEntry& directory) {
    return m_client->load_trading_group(directory);
  }

  template<typename C>
  std::vector<Beam::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_managed_trading_groups(const Beam::DirectoryEntry& account) {
    return m_client->load_managed_trading_groups(account);
  }

  template<typename C>
  std::vector<Beam::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_administrators() {
    return m_client->load_administrators();
  }

  template<typename C>
  std::vector<Beam::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::load_services() {
    return m_client->load_services();
  }

  template<typename C>
  EntitlementDatabase
      AdministrationClient::WrappedAdministrationClient<C>::
        load_entitlements() {
    return m_client->load_entitlements();
  }

  template<typename C>
  std::vector<Beam::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::load_entitlements(
        const Beam::DirectoryEntry& account) {
    return m_client->load_entitlements(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::
      store_entitlements(const Beam::DirectoryEntry& account,
        const std::vector<Beam::DirectoryEntry>& entitlements) {
    m_client->store_entitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskParameters>&
      AdministrationClient::WrappedAdministrationClient<C>::
        get_risk_parameters_publisher(const Beam::DirectoryEntry& account) {
    return m_client->get_risk_parameters_publisher(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::store(
      const Beam::DirectoryEntry& account, const RiskParameters& parameters) {
    m_client->store(account, parameters);
  }

  template<typename C>
  const Beam::Publisher<RiskState>&
      AdministrationClient::WrappedAdministrationClient<C>::
        get_risk_state_publisher(const Beam::DirectoryEntry& account) {
    return m_client->get_risk_state_publisher(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::store(
      const Beam::DirectoryEntry& account, const RiskState& state) {
    m_client->store(account, state);
  }

  template<typename C>
  AccountModificationRequest AdministrationClient::
      WrappedAdministrationClient<C>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_managed_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_managed_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename C>
  EntitlementModification AdministrationClient::
      WrappedAdministrationClient<C>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return m_client->load_entitlement_modification(id);
  }

  template<typename C>
  AccountModificationRequest AdministrationClient::
      WrappedAdministrationClient<C>::submit(
        const Beam::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return m_client->submit(account, modification, comment);
  }

  template<typename C>
  RiskModification AdministrationClient::WrappedAdministrationClient<C>::
      load_risk_modification(AccountModificationRequest::Id id) {
    return m_client->load_risk_modification(id);
  }

  template<typename C>
  AccountModificationRequest AdministrationClient::
      WrappedAdministrationClient<C>::submit(
        const Beam::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) {
    return m_client->submit(account, modification, comment);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClient::
      WrappedAdministrationClient<C>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request_status(id);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClient::
      WrappedAdministrationClient<C>::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->approve_account_modification_request(id, comment);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClient::
      WrappedAdministrationClient<C>::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->reject_account_modification_request(id, comment);
  }

  template<typename C>
  Message AdministrationClient::WrappedAdministrationClient<C>::load_message(
      Message::Id id) {
    return m_client->load_message(id);
  }

  template<typename C>
  std::vector<Message::Id> AdministrationClient::
      WrappedAdministrationClient<C>::load_message_ids(
        AccountModificationRequest::Id id) {
    return m_client->load_message_ids(id);
  }

  template<typename C>
  Message AdministrationClient::WrappedAdministrationClient<C>::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return m_client->send_account_modification_request_message(id, message);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::close() {
    m_client->close();
  }
}

#endif
