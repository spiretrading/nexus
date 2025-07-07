#ifndef NEXUS_CACHED_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_CACHED_ADMINISTRATION_DATA_STORE_HPP
#include <type_traits>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

namespace Nexus::AdministrationService {

  /**
   * Caches an AdministratorDataStore in memory.
   * @param <D> The type of AdministrationDataStore to cache.
   */
  template<typename D>
  class CachedAdministrationDataStore {
    public:
      using IndexedAccountIdentity =
        AdministrationDataStore::IndexedAccountIdentity;
      using IndexedRiskParameters =
        AdministrationDataStore::IndexedRiskParameters;
      using IndexedRiskState = AdministrationDataStore::IndexedRiskState;

      /** The type of AdministrationDataStore to cache. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedAdministrationDataStore.
       * @param data_store The AdministrationDataStore to cache.
       */
      template<typename S>
      CachedAdministrationDataStore(S&& data_store);
      ~CachedAdministrationDataStore();
      std::vector<IndexedAccountIdentity> load_all_account_identities();
      AccountIdentity load_identity(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);
      std::vector<IndexedRiskParameters> load_all_risk_parameters();
      RiskService::RiskParameters load_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& risk_parameters);
      std::vector<IndexedRiskState> load_all_risk_states();
      RiskService::RiskState load_risk_state(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& risk_state);
      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
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
      Beam::GetOptionalLocalPtr<D> m_data_store;
      LocalAdministrationDataStore m_cache;
  };

  template<typename D>
  template<typename S>
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
      BOOST_RETHROW;
    }
  }

  template<typename D>
  CachedAdministrationDataStore<D>::~CachedAdministrationDataStore() {
    close();
  }

  template<typename D>
  std::vector<typename CachedAdministrationDataStore<D>::IndexedAccountIdentity>
      CachedAdministrationDataStore<D>::load_all_account_identities() {
    return m_cache.load_all_account_identities();
  }

  template<typename D>
  AccountIdentity CachedAdministrationDataStore<D>::load_identity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.load_identity(account);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_data_store->store(account, identity);
    m_cache.store(account, identity);
  }

  template<typename D>
  std::vector<typename CachedAdministrationDataStore<D>::IndexedRiskParameters>
      CachedAdministrationDataStore<D>::load_all_risk_parameters() {
    return m_cache.load_all_risk_parameters();
  }

  template<typename D>
  RiskService::RiskParameters CachedAdministrationDataStore<D>::
      load_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.load_risk_parameters(account);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& risk_parameters) {
    m_data_store->store(account, risk_parameters);
    m_cache.store(account, risk_parameters);
  }

  template<typename D>
  std::vector<typename CachedAdministrationDataStore<D>::IndexedRiskState>
      CachedAdministrationDataStore<D>::load_all_risk_states() {
    return m_cache.load_all_risk_states();
  }

  template<typename D>
  RiskService::RiskState CachedAdministrationDataStore<D>::load_risk_state(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.load_risk_state(account);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& risk_state) {
    m_data_store->store(account, risk_state);
    m_cache.store(account, risk_state);
  }

  template<typename D>
  AccountModificationRequest CachedAdministrationDataStore<D>::
      load_account_modification_request(AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request(id);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id> CachedAdministrationDataStore<D>::
      load_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id>
      CachedAdministrationDataStore<D>::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    return m_data_store->load_account_modification_request_ids(
      start_id, max_count);
  }

  template<typename D>
  EntitlementModification CachedAdministrationDataStore<D>::
      load_entitlement_modification(AccountModificationRequest::Id id) {
    return m_data_store->load_entitlement_modification(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    m_data_store->store(request, modification);
  }

  template<typename D>
  RiskModification CachedAdministrationDataStore<D>::load_risk_modification(
      AccountModificationRequest::Id id) {
    return m_data_store->load_risk_modification(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    m_data_store->store(request, modification);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id, const Message& message) {
    m_data_store->store(id, message);
  }

  template<typename D>
  AccountModificationRequest::Update CachedAdministrationDataStore<D>::
      load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_data_store->load_account_modification_request_status(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    m_data_store->store(id, status);
  }

  template<typename D>
  Message::Id CachedAdministrationDataStore<D>::load_last_message_id() {
    return m_data_store->load_last_message_id();
  }

  template<typename D>
  Message CachedAdministrationDataStore<D>::load_message(Message::Id id) {
    return m_data_store->load_message(id);
  }

  template<typename D>
  std::vector<Message::Id> CachedAdministrationDataStore<D>::load_message_ids(
      AccountModificationRequest::Id id) {
    return m_data_store->load_message_ids(id);
  }

  template<typename D>
  template<typename F>
  decltype(auto) CachedAdministrationDataStore<D>::with_transaction(
      F&& transaction) {
    return m_data_store->with_transaction([&] {
      return m_cache.with_transaction(std::forward<F>(transaction));
    });
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::close() {
    m_data_store->close();
    m_cache.close();
  }
}

#endif
