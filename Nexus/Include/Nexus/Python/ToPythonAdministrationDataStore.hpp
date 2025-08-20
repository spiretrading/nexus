#ifndef NEXUS_TO_PYTHON_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_TO_PYTHON_ADMINISTRATION_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"

namespace Nexus::AdministrationService {

  /**
   * Wraps an AdministrationDataStore for use with Python.
   * @param <D> The type of AdministrationDataStore to wrap.
   */
  template<typename D>
  class ToPythonAdministrationDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonAdministrationDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename = Beam::disable_copy_constructor_t<
        ToPythonAdministrationDataStore, Args...>>
      ToPythonAdministrationDataStore(Args&&... args);

      ~ToPythonAdministrationDataStore();

      /** Returns the wrapped data store. */
      const DataStore& get_data_store() const;

      /** Returns the wrapped data store. */
      DataStore& get_data_store();

      std::vector<AdministrationDataStore::IndexedAccountIdentity>
        load_all_account_identities();
      AccountIdentity load_identity(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);
      std::vector<AdministrationDataStore::IndexedRiskParameters>
        load_all_risk_parameters();
      RiskService::RiskParameters load_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& risk_parameters);
      std::vector<AdministrationDataStore::IndexedRiskState>
        load_all_risk_states();
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
    ToPythonAdministrationDataStore<std::remove_reference_t<DataStore>>;

  template<typename D>
  template<typename... Args, typename>
  ToPythonAdministrationDataStore<D>::ToPythonAdministrationDataStore(
    Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename D>
  ToPythonAdministrationDataStore<D>::~ToPythonAdministrationDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<typename D>
  const typename ToPythonAdministrationDataStore<D>::DataStore&
      ToPythonAdministrationDataStore<D>::get_data_store() const {
    return *m_data_store;
  }

  template<typename D>
  typename ToPythonAdministrationDataStore<D>::DataStore&
      ToPythonAdministrationDataStore<D>::get_data_store() {
    return *m_data_store;
  }

  template<typename D>
  std::vector<AdministrationDataStore::IndexedAccountIdentity>
      ToPythonAdministrationDataStore<D>::load_all_account_identities() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_account_identities();
  }

  template<typename D>
  AccountIdentity ToPythonAdministrationDataStore<D>::load_identity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_identity(account);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, identity);
  }

  template<typename D>
  std::vector<AdministrationDataStore::IndexedRiskParameters>
      ToPythonAdministrationDataStore<D>::load_all_risk_parameters() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_risk_parameters();
  }

  template<typename D>
  RiskService::RiskParameters
      ToPythonAdministrationDataStore<D>::load_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_risk_parameters(account);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& risk_parameters) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, risk_parameters);
  }

  template<typename D>
  std::vector<AdministrationDataStore::IndexedRiskState>
      ToPythonAdministrationDataStore<D>::load_all_risk_states() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_risk_states();
  }

  template<typename D>
  RiskService::RiskState ToPythonAdministrationDataStore<D>::load_risk_state(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_risk_state(account);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& risk_state) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(account, risk_state);
  }

  template<typename D>
  AccountModificationRequest
      ToPythonAdministrationDataStore<D>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request(id);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationDataStore<D>::load_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationDataStore<D>::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request_ids(
      start_id, max_count);
  }

  template<typename D>
  EntitlementModification
      ToPythonAdministrationDataStore<D>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_entitlement_modification(id);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(request, modification);
  }

  template<typename D>
  RiskModification ToPythonAdministrationDataStore<D>::load_risk_modification(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_risk_modification(id);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(request, modification);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id, const Message& message) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(id, message);
  }

  template<typename D>
  AccountModificationRequest::Update ToPythonAdministrationDataStore<D>::
      load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_account_modification_request_status(id);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(id, status);
  }

  template<typename D>
  Message::Id ToPythonAdministrationDataStore<D>::load_last_message_id() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_last_message_id();
  }

  template<typename D>
  Message ToPythonAdministrationDataStore<D>::load_message(Message::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_message(id);
  }

  template<typename D>
  std::vector<Message::Id> ToPythonAdministrationDataStore<D>::load_message_ids(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_message_ids(id);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::with_transaction(
      const std::function<void ()>& transaction) {
    auto release = Beam::Python::GilRelease();
    m_data_store->with_transaction(transaction);
  }

  template<typename D>
  void ToPythonAdministrationDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
