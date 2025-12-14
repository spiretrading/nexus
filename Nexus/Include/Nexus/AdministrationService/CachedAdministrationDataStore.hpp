#ifndef NEXUS_CACHED_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_CACHED_ADMINISTRATION_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

namespace Nexus {

  /**
   * Caches an AdministratorDataStore in memory.
   * @param <D> The type of AdministrationDataStore to cache.
   */
  template<IsAdministrationDataStore D>
  class CachedAdministrationDataStore {
    public:
      using IndexedAccountIdentity =
        AdministrationDataStore::IndexedAccountIdentity;
      using IndexedRiskParameters =
        AdministrationDataStore::IndexedRiskParameters;
      using IndexedRiskState = AdministrationDataStore::IndexedRiskState;

      /** The type of AdministrationDataStore to cache. */
      using DataStore = Beam::dereference_t<D>;

      /**
       * Constructs a CachedAdministrationDataStore.
       * @param data_store The AdministrationDataStore to cache.
       */
      template<Beam::Initializes<D> S>
      CachedAdministrationDataStore(S&& data_store);

      ~CachedAdministrationDataStore();

      std::vector<IndexedAccountIdentity> load_all_account_identities();
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);
      std::vector<IndexedRiskParameters> load_all_risk_parameters();
      RiskParameters load_risk_parameters(const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account,
        const RiskParameters& risk_parameters);
      std::vector<IndexedRiskState> load_all_risk_states();
      RiskState load_risk_state(const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const RiskState& risk_state);
      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          AccountModificationRequest::Id start_id, int max_count);
      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);
      void store(const AccountModificationRequest& request,
        const EntitlementModification& modification);
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);
      void store(const AccountModificationRequest& request,
        const RiskModification& modification);
      void store(AccountModificationRequest::Id id, const Message& message);
      AccountModificationRequest::Update
        load_account_modification_request_status(
          AccountModificationRequest::Id id);
      void store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status);
      Message::Id load_last_message_id();
      Message load_message(Message::Id id);
      std::vector<Message::Id> load_message_ids(
        AccountModificationRequest::Id id);
      template<typename F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      Beam::local_ptr_t<D> m_data_store;
      LocalAdministrationDataStore m_cache;
  };

  template<IsAdministrationDataStore D>
  template<Beam::Initializes<D> S>
  CachedAdministrationDataStore<D>::CachedAdministrationDataStore(
      S&& data_store)
      : m_data_store(std::forward<S>(data_store)) {
    try {
      auto identities = m_data_store->load_all_account_identities();
      for(auto& identity : identities) {
        m_cache.store(identity.m_index, identity.m_identity);
      }
      auto risk_parameters = m_data_store->load_all_risk_parameters();
      for(auto& risk_parameter : risk_parameters) {
        m_cache.store(risk_parameter.m_index, risk_parameter.m_parameters);
      }
      auto risk_states = m_data_store->load_all_risk_states();
      for(auto& risk_state : risk_states) {
        m_cache.store(risk_state.m_index, risk_state.m_state);
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<IsAdministrationDataStore D>
  CachedAdministrationDataStore<D>::~CachedAdministrationDataStore() {
    close();
  }

  template<IsAdministrationDataStore D>
  std::vector<typename CachedAdministrationDataStore<D>::IndexedAccountIdentity>
      CachedAdministrationDataStore<D>::load_all_account_identities() {
    return m_cache.load_all_account_identities();
  }

  template<IsAdministrationDataStore D>
  AccountIdentity CachedAdministrationDataStore<D>::load_identity(
      const Beam::DirectoryEntry& account) {
    return m_cache.load_identity(account);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    m_data_store->store(account, identity);
    m_cache.store(account, identity);
  }

  template<IsAdministrationDataStore D>
  std::vector<typename CachedAdministrationDataStore<D>::IndexedRiskParameters>
      CachedAdministrationDataStore<D>::load_all_risk_parameters() {
    return m_cache.load_all_risk_parameters();
  }

  template<IsAdministrationDataStore D>
  RiskParameters CachedAdministrationDataStore<D>::load_risk_parameters(
      const Beam::DirectoryEntry& account) {
    return m_cache.load_risk_parameters(account);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    m_data_store->store(account, risk_parameters);
    m_cache.store(account, risk_parameters);
  }

  template<IsAdministrationDataStore D>
  std::vector<typename CachedAdministrationDataStore<D>::IndexedRiskState>
      CachedAdministrationDataStore<D>::load_all_risk_states() {
    return m_cache.load_all_risk_states();
  }

  template<IsAdministrationDataStore D>
  RiskState CachedAdministrationDataStore<D>::load_risk_state(
      const Beam::DirectoryEntry& account) {
    return m_cache.load_risk_state(account);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account, const RiskState& risk_state) {
    m_data_store->store(account, risk_state);
    m_cache.store(account, risk_state);
  }

  template<IsAdministrationDataStore D>
  AccountModificationRequest CachedAdministrationDataStore<D>::
      load_account_modification_request(AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request(id);
  }

  template<IsAdministrationDataStore D>
  std::vector<AccountModificationRequest::Id> CachedAdministrationDataStore<D>::
      load_account_modification_request_ids(const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<IsAdministrationDataStore D>
  std::vector<AccountModificationRequest::Id>
      CachedAdministrationDataStore<D>::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      start_id, max_count);
  }

  template<IsAdministrationDataStore D>
  EntitlementModification CachedAdministrationDataStore<D>::
      load_entitlement_modification(AccountModificationRequest::Id id) {
    return m_data_store->load_entitlement_modification(id);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    m_data_store->store(request, modification);
  }

  template<IsAdministrationDataStore D>
  RiskModification CachedAdministrationDataStore<D>::load_risk_modification(
      AccountModificationRequest::Id id) {
    return m_data_store->load_risk_modification(id);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    m_data_store->store(request, modification);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id, const Message& message) {
    m_data_store->store(id, message);
  }

  template<IsAdministrationDataStore D>
  AccountModificationRequest::Update CachedAdministrationDataStore<D>::
      load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request_status(id);
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    m_data_store->store(id, status);
  }

  template<IsAdministrationDataStore D>
  Message::Id CachedAdministrationDataStore<D>::load_last_message_id() {
    return m_data_store->load_last_message_id();
  }

  template<IsAdministrationDataStore D>
  Message CachedAdministrationDataStore<D>::load_message(Message::Id id) {
    return m_data_store->load_message(id);
  }

  template<IsAdministrationDataStore D>
  std::vector<Message::Id> CachedAdministrationDataStore<D>::load_message_ids(
      AccountModificationRequest::Id id) {
    return m_data_store->load_message_ids(id);
  }

  template<IsAdministrationDataStore D>
  template<typename F>
  decltype(auto) CachedAdministrationDataStore<D>::with_transaction(
      F&& transaction) {
    return m_data_store->with_transaction([&] {
      return m_cache.with_transaction(std::forward<F>(transaction));
    });
  }

  template<IsAdministrationDataStore D>
  void CachedAdministrationDataStore<D>::close() {
    m_data_store->close();
    m_cache.close();
  }
}

#endif
