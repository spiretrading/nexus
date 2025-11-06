#ifndef NEXUS_TO_PYTHON_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_TO_PYTHON_ADMINISTRATION_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"

namespace Nexus {

  /**
   * Wraps an AdministrationDataStore for use with Python.
   * @param <D> The type of AdministrationDataStore to wrap.
   */
  template<IsAdministrationDataStore D>
  class ToPythonAdministrationDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonAdministrationDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonAdministrationDataStore(Args&&... args);

      ~ToPythonAdministrationDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      /** Returns a reference to the underlying data store. */
      DataStore& operator *();

      /** Returns a reference to the underlying data store. */
      const DataStore& operator *() const;

      /** Returns a pointer to the underlying data store. */
      DataStore* operator ->();

      /** Returns a pointer to the underlying data store. */
      const DataStore* operator ->() const;

      std::vector<AdministrationDataStore::IndexedAccountIdentity>
        load_all_account_identities();
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);
      std::vector<AdministrationDataStore::IndexedRiskParameters>
        load_all_risk_parameters();
      RiskParameters load_risk_parameters(const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account,
        const RiskParameters& risk_parameters);
      std::vector<AdministrationDataStore::IndexedRiskState>
        load_all_risk_states();
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
      void with_transaction(const std::function<void ()>& transaction);
      void close();

    private:
      boost::optional<DataStore> m_data_store;

      ToPythonAdministrationDataStore(
        const ToPythonAdministrationDataStore&) = delete;
      ToPythonAdministrationDataStore& operator =(
        const ToPythonAdministrationDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonAdministrationDataStore(DataStore&&) ->
    ToPythonAdministrationDataStore<std::remove_cvref_t<DataStore>>;

  template<IsAdministrationDataStore D>
  template<typename... Args>
  ToPythonAdministrationDataStore<D>::ToPythonAdministrationDataStore(
    Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsAdministrationDataStore D>
  ToPythonAdministrationDataStore<D>::~ToPythonAdministrationDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsAdministrationDataStore D>
  typename ToPythonAdministrationDataStore<D>::DataStore&
      ToPythonAdministrationDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsAdministrationDataStore D>
  const typename ToPythonAdministrationDataStore<D>::DataStore&
      ToPythonAdministrationDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsAdministrationDataStore D>
  typename ToPythonAdministrationDataStore<D>::DataStore&
      ToPythonAdministrationDataStore<D>::operator *() {
    return *m_data_store;
  }

  template<IsAdministrationDataStore D>
  const typename ToPythonAdministrationDataStore<D>::DataStore&
      ToPythonAdministrationDataStore<D>::operator *() const {
    return *m_data_store;
  }

  template<IsAdministrationDataStore D>
  typename ToPythonAdministrationDataStore<D>::DataStore*
      ToPythonAdministrationDataStore<D>::operator ->() {
    return m_data_store.get_ptr();
  }

  template<IsAdministrationDataStore D>
  const typename ToPythonAdministrationDataStore<D>::DataStore*
      ToPythonAdministrationDataStore<D>::operator ->() const {
    return m_data_store.get_ptr();
  }

  template<IsAdministrationDataStore D>
  std::vector<AdministrationDataStore::IndexedAccountIdentity>
      ToPythonAdministrationDataStore<D>::load_all_account_identities() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_account_identities();
  }

  template<IsAdministrationDataStore D>
  AccountIdentity ToPythonAdministrationDataStore<D>::load_identity(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_identity(account);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, identity);
  }

  template<IsAdministrationDataStore D>
  std::vector<AdministrationDataStore::IndexedRiskParameters>
      ToPythonAdministrationDataStore<D>::load_all_risk_parameters() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_risk_parameters();
  }

  template<IsAdministrationDataStore D>
  RiskParameters ToPythonAdministrationDataStore<D>::load_risk_parameters(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_risk_parameters(account);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, risk_parameters);
  }

  template<IsAdministrationDataStore D>
  std::vector<AdministrationDataStore::IndexedRiskState>
      ToPythonAdministrationDataStore<D>::load_all_risk_states() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_risk_states();
  }

  template<IsAdministrationDataStore D>
  RiskState ToPythonAdministrationDataStore<D>::load_risk_state(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_risk_state(account);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      const Beam::DirectoryEntry& account, const RiskState& risk_state) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, risk_state);
  }

  template<IsAdministrationDataStore D>
  AccountModificationRequest
      ToPythonAdministrationDataStore<D>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request(id);
  }

  template<IsAdministrationDataStore D>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationDataStore<D>::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<IsAdministrationDataStore D>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationDataStore<D>::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request_ids(
      start_id, max_count);
  }

  template<IsAdministrationDataStore D>
  EntitlementModification
      ToPythonAdministrationDataStore<D>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_entitlement_modification(id);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(request, modification);
  }

  template<IsAdministrationDataStore D>
  RiskModification ToPythonAdministrationDataStore<D>::load_risk_modification(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_risk_modification(id);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(request, modification);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id, const Message& message) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(id, message);
  }

  template<IsAdministrationDataStore D>
  AccountModificationRequest::Update ToPythonAdministrationDataStore<D>::
      load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request_status(id);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(id, status);
  }

  template<IsAdministrationDataStore D>
  Message::Id ToPythonAdministrationDataStore<D>::load_last_message_id() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_last_message_id();
  }

  template<IsAdministrationDataStore D>
  Message ToPythonAdministrationDataStore<D>::load_message(Message::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_message(id);
  }

  template<IsAdministrationDataStore D>
  std::vector<Message::Id> ToPythonAdministrationDataStore<D>::load_message_ids(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_message_ids(id);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::with_transaction(
      const std::function<void ()>& transaction) {
    auto release = Beam::Python::GilRelease();
    m_data_store->with_transaction(transaction);
  }

  template<IsAdministrationDataStore D>
  void ToPythonAdministrationDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
