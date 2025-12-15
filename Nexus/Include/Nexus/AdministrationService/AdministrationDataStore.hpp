#ifndef NEXUS_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_ADMINISTRATION_DATA_STORE_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/optional/optional.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {

  /** Concept for types that can be used as an administration data store. */
  template<typename T>
  concept IsAdministrationDataStore = Beam::IsConnection<T> && requires(
      T& store) {
    store.load_all_account_identities();
    { store.load_identity(std::declval<const Beam::DirectoryEntry&>()) } ->
        std::same_as<AccountIdentity>;
    store.store(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const AccountIdentity&>());
    store.load_all_risk_parameters();
    { store.load_risk_parameters(
        std::declval<const Beam::DirectoryEntry&>()) } ->
          std::same_as<RiskParameters>;
    store.store(std::declval<const Beam::DirectoryEntry&>(),
      std::declval<const RiskParameters&>());
    store.load_all_risk_states();
    { store.load_risk_state(std::declval<const Beam::DirectoryEntry&>()) } ->
        std::same_as<RiskState>;
    store.store(std::declval<const Beam::DirectoryEntry&>(),
      std::declval<const RiskState&>());
    { store.load_account_modification_request(
      std::declval<AccountModificationRequest::Id>()) } ->
        std::same_as<AccountModificationRequest>;
    { store.load_account_modification_request_ids(
        std::declval<const Beam::DirectoryEntry&>(),
        std::declval<AccountModificationRequest::Id>(),
        std::declval<int>()) } ->
          std::same_as<std::vector<AccountModificationRequest::Id>>;
    { store.load_account_modification_request_ids(
      std::declval<AccountModificationRequest::Id>(), std::declval<int>()) } ->
      std::same_as<std::vector<AccountModificationRequest::Id>>;
    { store.load_entitlement_modification(
      std::declval<AccountModificationRequest::Id>()) } ->
      std::same_as<EntitlementModification>;
    store.store(std::declval<const AccountModificationRequest&>(),
      std::declval<const EntitlementModification&>());
    { store.load_risk_modification(
      std::declval<AccountModificationRequest::Id>()) } ->
      std::same_as<RiskModification>;
    store.store(std::declval<const AccountModificationRequest&>(),
      std::declval<const RiskModification&>());
    store.store(std::declval<AccountModificationRequest::Id>(),
      std::declval<const Message&>());
    { store.load_account_modification_request_status(
      std::declval<AccountModificationRequest::Id>()) } ->
      std::same_as<AccountModificationRequest::Update>;
    store.store(std::declval<AccountModificationRequest::Id>(),
      std::declval<const AccountModificationRequest::Update&>());
    { store.load_last_message_id() } -> std::same_as<Message::Id>;
    { store.load_message(std::declval<Message::Id>()) } ->
      std::same_as<Message>;
    { store.load_message_ids(
      std::declval<AccountModificationRequest::Id>()) } ->
      std::same_as<std::vector<Message::Id>>;
    { store.with_transaction(
      std::declval<const std::function<void ()>&>()) } -> std::same_as<void>;
  };

  /** Provides a generic interface over an arbitrary AdministrationDataStore. */
  class AdministrationDataStore {
    public:

      /**
       * Stores an AccountIdentity and its index.
       * Used to associate an account's identity with its directory entry.
       */
      struct IndexedAccountIdentity {

        /** The directory entry representing the account. */
        Beam::DirectoryEntry m_index;

        /** The identity associated with the account. */
        AccountIdentity m_identity;
      };

      /**
       * Stores RiskParameters and the account they are associated with.
       * Used to associate risk parameters with a specific account.
       */
      struct IndexedRiskParameters {

        /** The directory entry representing the account. */
        Beam::DirectoryEntry m_index;

        /** The risk parameters associated with the account. */
        RiskParameters m_parameters;
      };

      /**
       * Stores a RiskState and the account it is associated with.
       * Used to associate a risk state with a specific account.
       */
      struct IndexedRiskState {

        /** The directory entry representing the account. */
        Beam::DirectoryEntry m_index;

        /** The risk state associated with the account. */
        RiskState m_state;
      };

      /**
       * Constructs an AdministrationDataStore of a specified type using
       * emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsAdministrationDataStore T, typename... Args>
      explicit AdministrationDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an AdministrationDataStore by referencing an existing data
       * store.
       * @param data_store The data store to reference.
       */
      template<Beam::DisableCopy<AdministrationDataStore> T> requires
        IsAdministrationDataStore<Beam::dereference_t<T>>
      AdministrationDataStore(T&& data_store);

      AdministrationDataStore(const AdministrationDataStore&) = default;
      AdministrationDataStore(AdministrationDataStore&&) = default;

      /** Loads all AccountIdentities. */
      std::vector<IndexedAccountIdentity> load_all_account_identities();

      /**
       * Loads an AccountIdentity.
       * @param account The account whose identity is to be loaded.
       * @return The AccountIdentity of the specified account.
       */
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);

      /**
       * Stores an AccountIdentity.
       * @param account The account of the identity to store.
       * @param identity The AccountIdentity to store.
       */
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);

      /** Loads all RiskParameters. */
      std::vector<IndexedRiskParameters> load_all_risk_parameters();

      /**
       * Loads an account's RiskParameters.
       * @param account The account whose RiskParameters are to be loaded.
       * @return The account's RiskParameters.
       */
      RiskParameters load_risk_parameters(const Beam::DirectoryEntry& account);

      /**
       * Stores an account's RiskParameters.
       * @param account The account of the RiskParameters to store.
       * @param risk_parameters The RiskParameters to store for the account.
       */
      void store(const Beam::DirectoryEntry& account,
        const RiskParameters& risk_parameters);

      /** Loads all RiskStates. */
      std::vector<IndexedRiskState> load_all_risk_states();

      /**
       * Loads an account's RiskState.
       * @param account The account whose RiskState is to be loaded.
       * @return The account's RiskState.
       */
      RiskState load_risk_state(const Beam::DirectoryEntry& account);

      /**
       * Stores an account's RiskState.
       * @param account The account of the RiskState to store.
       * @param risk_state The RiskState to store for the account.
       */
      void store(
        const Beam::DirectoryEntry& account, const RiskState& risk_state);

      /**
       * Loads an AccountModificationRequest.
       * @param id The id of the request to load.
       * @return The AccountModificationRequest with the specified id.
       */
      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);

      /**
       * Loads all account modification request ids to modify an account.
       * @param account The account being modified.
       * @param start_id The id of the most recent request to load (exclusive),
       *        or -1 to start with the most recent request.
       * @param max_count The maximum number of ids to load.
       * @return The list of ids specified.
       */
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);

      /**
       * Loads all account modification request ids.
       * @param start_id The id of the most recent request to load (exclusive),
       *        or -1 to start with the most recent request.
       * @param max_count The maximum number of ids to load.
       * @return The list of ids specified.
       */
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          AccountModificationRequest::Id start_id, int max_count);

      /**
       * Loads an EntitlementModification.
       * @param id The id of the request to load.
       * @return The EntitlementModification with the specified id.
       */
      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);

      /**
       * Stores an EntitlementModification.
       * @param request The modification request.
       * @param modification The details of the modification.
       */
      void store(const AccountModificationRequest& request,
        const EntitlementModification& modification);

      /**
       * Loads a RiskModification.
       * @param id The id of the request to load.
       * @return The RiskModification with the specified id.
       */
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);

      /**
       * Stores a RiskModification.
       * @param request The modification request.
       * @param modification The details of the modification.
       */
      void store(const AccountModificationRequest& request,
        const RiskModification& modification);

      /**
       * Stores a Message associated with an account modification request.
       * @param id The id of the request.
       * @param message The Message to store.
       */
      void store(AccountModificationRequest::Id id, const Message& message);

      /**
       * Loads the status of an AccountModificationRequest.
       * @param id The id of the request.
       * @return The update containing the current status of the request with
       *         the specified id.
       */
      AccountModificationRequest::Update
        load_account_modification_request_status(
          AccountModificationRequest::Id id);

      /**
       * Stores the status of an AccountModificationRequest.
       * @param id The id of the request.
       * @param status The update containing the current status.
       */
      void store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status);

      /**
       * Loads the id of the last Message stored, or 0 if there are no stored
       * Messages.
       * @return The id of the last Message stored.
       */
      Message::Id load_last_message_id();

      /**
       * Loads a Message.
       * @param id The id of the Message to load.
       * @return The Message with the specified id.
       */
      Message load_message(Message::Id id);

      /**
       * Loads Message ids belonging to an AccountModificationRequest.
       * @param id The id of the AccountModificationRequest.
       * @return The list of Message ids belonging to the
       *         AccountModificationRequest with the specified id.
       */
      std::vector<Message::Id> load_message_ids(
        AccountModificationRequest::Id id);

      /**
       * Performs an atomic transaction.
       * @param transaction The transaction to perform.
       */
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);

      /** Closes the data store. */
      void close();

    private:
      struct VirtualAdministrationDataStore {
        virtual ~VirtualAdministrationDataStore() = default;

        virtual std::vector<IndexedAccountIdentity>
          load_all_account_identities() = 0;
        virtual AccountIdentity load_identity(
          const Beam::DirectoryEntry& account) = 0;
        virtual void store(const Beam::DirectoryEntry& account,
          const AccountIdentity& identity) = 0;
        virtual std::vector<IndexedRiskParameters>
          load_all_risk_parameters() = 0;
        virtual RiskParameters load_risk_parameters(
          const Beam::DirectoryEntry& account) = 0;
        virtual void store(const Beam::DirectoryEntry& account,
          const RiskParameters& risk_parameters) = 0;
        virtual std::vector<IndexedRiskState> load_all_risk_states() = 0;
        virtual RiskState load_risk_state(
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
          load_account_modification_request_ids(
            AccountModificationRequest::Id start_id, int max_count) = 0;
        virtual EntitlementModification load_entitlement_modification(
          AccountModificationRequest::Id id) = 0;
        virtual void store(const AccountModificationRequest& request,
          const EntitlementModification& modification) = 0;
        virtual RiskModification load_risk_modification(
          AccountModificationRequest::Id id) = 0;
        virtual void store(const AccountModificationRequest& request,
          const RiskModification& modification) = 0;
        virtual void store(
          AccountModificationRequest::Id id, const Message& message) = 0;
        virtual AccountModificationRequest::Update
          load_account_modification_request_status(
            AccountModificationRequest::Id id) = 0;
        virtual void store(AccountModificationRequest::Id id,
          const AccountModificationRequest::Update& status) = 0;
        virtual Message::Id load_last_message_id() = 0;
        virtual Message load_message(Message::Id id) = 0;
        virtual std::vector<Message::Id> load_message_ids(
          AccountModificationRequest::Id id) = 0;
        virtual void with_transaction(
          const std::function<void ()>& transaction) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedAdministrationDataStore final :
          VirtualAdministrationDataStore {
        using DataStore = D;
        Beam::local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedAdministrationDataStore(Args&&... args);

        std::vector<IndexedAccountIdentity>
          load_all_account_identities() override;
        AccountIdentity load_identity(
          const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const AccountIdentity& identity) override;
        std::vector<IndexedRiskParameters> load_all_risk_parameters() override;
        RiskParameters load_risk_parameters(
          const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const RiskParameters& risk_parameters) override;
        std::vector<IndexedRiskState> load_all_risk_states() override;
        RiskState load_risk_state(const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const RiskState& risk_state) override;
        AccountModificationRequest load_account_modification_request(
          AccountModificationRequest::Id id) override;
        std::vector<AccountModificationRequest::Id>
          load_account_modification_request_ids(
            const Beam::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) override;
        std::vector<AccountModificationRequest::Id>
          load_account_modification_request_ids(
            AccountModificationRequest::Id start_id, int max_count) override;
        EntitlementModification load_entitlement_modification(
          AccountModificationRequest::Id id) override;
        void store(const AccountModificationRequest& request,
          const EntitlementModification& modification) override;
        RiskModification load_risk_modification(
          AccountModificationRequest::Id id) override;
        void store(const AccountModificationRequest& request,
          const RiskModification& modification) override;
        void store(
          AccountModificationRequest::Id id, const Message& message) override;
        AccountModificationRequest::Update
          load_account_modification_request_status(
            AccountModificationRequest::Id id) override;
        void store(AccountModificationRequest::Id id,
          const AccountModificationRequest::Update& status) override;
        Message::Id load_last_message_id() override;
        Message load_message(Message::Id id) override;
        std::vector<Message::Id> load_message_ids(
          AccountModificationRequest::Id id) override;
        void with_transaction(
          const std::function<void ()>& transaction) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualAdministrationDataStore> m_data_store;
  };

  template<IsAdministrationDataStore T, typename... Args>
  AdministrationDataStore::AdministrationDataStore(std::in_place_type_t<T>,
      Args&&... args)
    : m_data_store(Beam::make_virtual_ptr<WrappedAdministrationDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<AdministrationDataStore> T> requires
    IsAdministrationDataStore<Beam::dereference_t<T>>
  AdministrationDataStore::AdministrationDataStore(T&& data_store)
    : m_data_store(Beam::make_virtual_ptr<WrappedAdministrationDataStore<
        std::remove_cvref_t<T>>>(std::forward<T>(data_store))) {}

  inline std::vector<AdministrationDataStore::IndexedAccountIdentity>
      AdministrationDataStore::load_all_account_identities() {
    return m_data_store->load_all_account_identities();
  }

  inline AccountIdentity AdministrationDataStore::load_identity(
      const Beam::DirectoryEntry& account) {
    return m_data_store->load_identity(account);
  }

  inline void AdministrationDataStore::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    m_data_store->store(account, identity);
  }

  inline std::vector<AdministrationDataStore::IndexedRiskParameters>
      AdministrationDataStore::load_all_risk_parameters() {
    return m_data_store->load_all_risk_parameters();
  }

  inline RiskParameters AdministrationDataStore::load_risk_parameters(
      const Beam::DirectoryEntry& account) {
    return m_data_store->load_risk_parameters(account);
  }

  inline void AdministrationDataStore::store(
      const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    m_data_store->store(account, risk_parameters);
  }

  inline std::vector<AdministrationDataStore::IndexedRiskState>
      AdministrationDataStore::load_all_risk_states() {
    return m_data_store->load_all_risk_states();
  }

  inline RiskState AdministrationDataStore::load_risk_state(
      const Beam::DirectoryEntry& account) {
    return m_data_store->load_risk_state(account);
  }

  inline void AdministrationDataStore::store(
      const Beam::DirectoryEntry& account, const RiskState& risk_state) {
    m_data_store->store(account, risk_state);
  }

  inline AccountModificationRequest
      AdministrationDataStore::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request(id);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationDataStore::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationDataStore::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      start_id, max_count);
  }

  inline EntitlementModification
      AdministrationDataStore::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return m_data_store->load_entitlement_modification(id);
  }

  inline void AdministrationDataStore::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    m_data_store->store(request, modification);
  }

  inline RiskModification AdministrationDataStore::load_risk_modification(
      AccountModificationRequest::Id id) {
    return m_data_store->load_risk_modification(id);
  }

  inline void AdministrationDataStore::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    m_data_store->store(request, modification);
  }

  inline void AdministrationDataStore::store(
      AccountModificationRequest::Id id, const Message& message) {
    m_data_store->store(id, message);
  }

  inline AccountModificationRequest::Update
      AdministrationDataStore::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request_status(id);
  }

  inline void AdministrationDataStore::store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status) {
    m_data_store->store(id, status);
  }

  inline Message::Id AdministrationDataStore::load_last_message_id() {
    return m_data_store->load_last_message_id();
  }

  inline Message AdministrationDataStore::load_message(Message::Id id) {
    return m_data_store->load_message(id);
  }

  inline std::vector<Message::Id> AdministrationDataStore::load_message_ids(
      AccountModificationRequest::Id id) {
    return m_data_store->load_message_ids(id);
  }

  template<std::invocable<> F>
  decltype(auto) AdministrationDataStore::with_transaction(F&& transaction) {
    using R = std::invoke_result_t<F>;
    if constexpr(std::is_reference_v<R>) {
      auto result = static_cast<std::remove_reference_t<R>*>(nullptr);
      m_data_store->with_transaction([&] {
        result = &(std::forward<F>(transaction)());
      });
      return *result;
    } else if constexpr(std::is_void_v<R>) {
      m_data_store->with_transaction(std::forward<F>(transaction));
    } else {
      auto result = boost::optional<R>();
      m_data_store->with_transaction([&] {
        result.emplace(std::forward<F>(transaction)());
      });
      return R(std::move(*result));
    }
  }

  inline void AdministrationDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  AdministrationDataStore::WrappedAdministrationDataStore<D>::
    WrappedAdministrationDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<AdministrationDataStore::IndexedAccountIdentity>
      AdministrationDataStore::
        WrappedAdministrationDataStore<D>::load_all_account_identities() {
    return m_data_store->load_all_account_identities();
  }

  template<typename D>
  AccountIdentity AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_identity(
        const Beam::DirectoryEntry& account) {
    return m_data_store->load_identity(account);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    m_data_store->store(account, identity);
  }

  template<typename D>
  std::vector<AdministrationDataStore::IndexedRiskParameters>
      AdministrationDataStore::
        WrappedAdministrationDataStore<D>::load_all_risk_parameters() {
    return m_data_store->load_all_risk_parameters();
  }

  template<typename D>
  RiskParameters AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_risk_parameters(
        const Beam::DirectoryEntry& account) {
    return m_data_store->load_risk_parameters(account);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    m_data_store->store(account, risk_parameters);
  }

  template<typename D>
  std::vector<AdministrationDataStore::IndexedRiskState>
      AdministrationDataStore::
        WrappedAdministrationDataStore<D>::load_all_risk_states() {
    return m_data_store->load_all_risk_states();
  }

  template<typename D>
  RiskState AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_risk_state(
        const Beam::DirectoryEntry& account) {
    return m_data_store->load_risk_state(account);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account, const RiskState& risk_state) {
    m_data_store->store(account, risk_state);
  }

  template<typename D>
  AccountModificationRequest AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request(id);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id> AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id> AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      start_id, max_count);
  }

  template<typename D>
  EntitlementModification AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return m_data_store->load_entitlement_modification(id);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    m_data_store->store(request, modification);
  }

  template<typename D>
  RiskModification AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_risk_modification(
        AccountModificationRequest::Id id) {
    return m_data_store->load_risk_modification(id);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    m_data_store->store(request, modification);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id, const Message& message) {
    m_data_store->store(id, message);
  }

  template<typename D>
  AccountModificationRequest::Update AdministrationDataStore::
      WrappedAdministrationDataStore<D>::
        load_account_modification_request_status(
          AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request_status(id);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    m_data_store->store(id, status);
  }

  template<typename D>
  Message::Id AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_last_message_id() {
    return m_data_store->load_last_message_id();
  }

  template<typename D>
  Message AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_message(Message::Id id) {
    return m_data_store->load_message(id);
  }

  template<typename D>
  std::vector<Message::Id> AdministrationDataStore::
      WrappedAdministrationDataStore<D>::load_message_ids(
        AccountModificationRequest::Id id) {
    return m_data_store->load_message_ids(id);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::
      with_transaction(const std::function<void ()>& transaction) {
    m_data_store->with_transaction(transaction);
  }

  template<typename D>
  void AdministrationDataStore::WrappedAdministrationDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
