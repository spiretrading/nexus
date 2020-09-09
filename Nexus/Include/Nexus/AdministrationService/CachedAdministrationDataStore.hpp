#ifndef NEXUS_CACHED_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_CACHED_ADMINISTRATION_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"

namespace Nexus::AdministrationService {

  /**
   * Caches an AdministratorDataStore in memory.
   * @param <D> The type of AdministrationDataStore to cache.
   */
  template<typename D>
  class CachedAdministrationDataStore : public AdministrationDataStore {
    public:

      /** The type of ComplianceRuleDataStore to cache. */
      using DataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedAdministrationDataStore.
       * @param dataStore The AdministrationDataStore to cache.
       */
      template<typename DF>
      CachedAdministrationDataStore(DF&& dataStore);

      ~CachedAdministrationDataStore() override;

      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        AccountIdentity>> LoadAllAccountIdentities() override;

      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters>> LoadAllRiskParameters() override;

      RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState>> LoadAllRiskStates() override;

      RiskService::RiskState LoadRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override;

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        AccountModificationRequest::Id startId, int maxCount) override;

      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override;

      void Store(const AccountModificationRequest& request,
        const EntitlementModification& modification) override;

      RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) override;

      void Store(const AccountModificationRequest& request,
        const RiskModification& modification) override;

      void Store(AccountModificationRequest::Id id,
        const Message& message) override;

      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      void Store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status) override;

      Message::Id LoadLastMessageId() override;

      Message LoadMessage(Message::Id id) override;

      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      LocalAdministrationDataStore m_cache;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename D>
  template<typename DF>
  CachedAdministrationDataStore<D>::CachedAdministrationDataStore(
      DF&& dataStore)
      : m_dataStore(std::forward<DF>(dataStore)) {
    m_openState.SetOpening();
    try {
      auto identities = m_dataStore->LoadAllAccountIdentities();
      for(auto& identity : identities) {
        m_cache.Store(std::get<0>(identity), std::get<1>(identity));
      }
      auto riskParameters = m_dataStore->LoadAllRiskParameters();
      for(auto& riskParameter : riskParameters) {
        m_cache.Store(std::get<0>(riskParameter), std::get<1>(riskParameter));
      }
      auto riskStates = m_dataStore->LoadAllRiskStates();
      for(auto& riskState : riskStates) {
        m_cache.Store(std::get<0>(riskState), std::get<1>(riskState));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename D>
  CachedAdministrationDataStore<D>::~CachedAdministrationDataStore() {
    Close();
  }

  template<typename D>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>>
      CachedAdministrationDataStore<D>::LoadAllAccountIdentities() {
    return m_cache.LoadAllAccountIdentities();
  }

  template<typename D>
  AccountIdentity CachedAdministrationDataStore<D>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.LoadIdentity(account);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_dataStore->Store(account, identity);
    m_cache.Store(account, identity);
  }

  template<typename D>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>>
      CachedAdministrationDataStore<D>::LoadAllRiskParameters() {
    return m_cache.LoadAllRiskParameters();
  }

  template<typename D>
  RiskService::RiskParameters CachedAdministrationDataStore<D>::
      LoadRiskParameters(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.LoadRiskParameters(account);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_dataStore->Store(account, riskParameters);
    m_cache.Store(account, riskParameters);
  }

  template<typename D>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> CachedAdministrationDataStore<D>::
      LoadAllRiskStates() {
    return m_cache.LoadAllRiskStates();
  }

  template<typename D>
  RiskService::RiskState CachedAdministrationDataStore<D>::
      LoadRiskState(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_cache.LoadRiskState(account);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_dataStore->Store(account, riskState);
    m_cache.Store(account, riskState);
  }

  template<typename D>
  AccountModificationRequest CachedAdministrationDataStore<D>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    return m_dataStore->LoadAccountModificationRequest(id);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id> CachedAdministrationDataStore<
      D>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    return m_dataStore->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename D>
  std::vector<AccountModificationRequest::Id>
      CachedAdministrationDataStore<D>::LoadAccountModificationRequestIds(
      AccountModificationRequest::Id startId, int maxCount) {
    return m_dataStore->LoadAccountModificationRequestIds(startId, maxCount);
  }

  template<typename D>
  EntitlementModification CachedAdministrationDataStore<D>::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    return m_dataStore->LoadEntitlementModification(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    m_dataStore->Store(request, modification);
  }

  template<typename D>
  RiskModification CachedAdministrationDataStore<D>::LoadRiskModification(
      AccountModificationRequest::Id id) {
    return m_dataStore->LoadRiskModification(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    m_dataStore->Store(request, modification);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      AccountModificationRequest::Id id, const Message& message) {
    m_dataStore->Store(id, message);
  }

  template<typename D>
  AccountModificationRequest::Update
      CachedAdministrationDataStore<D>::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    return m_dataStore->LoadAccountModificationRequestStatus(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    m_dataStore->Store(id, status);
  }

  template<typename D>
  Message::Id CachedAdministrationDataStore<D>::LoadLastMessageId() {
    return m_dataStore->LoadLastMessageId();
  }

  template<typename D>
  Message CachedAdministrationDataStore<D>::LoadMessage(Message::Id id) {
    return m_dataStore->LoadMessage(id);
  }

  template<typename D>
  std::vector<Message::Id> CachedAdministrationDataStore<D>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    return m_dataStore->LoadMessageIds(id);
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::WithTransaction(
      const std::function<void ()>& transaction) {
    m_dataStore->WithTransaction(
      [&] {
        m_cache.WithTransaction(transaction);
      });
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename D>
  void CachedAdministrationDataStore<D>::Shutdown() {
    m_dataStore->Close();
    m_cache.Close();
    m_openState.SetClosed();
  }
}

#endif
