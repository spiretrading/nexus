#ifndef NEXUS_LOCALADMINISTRATIONDATASTORE_HPP
#define NEXUS_LOCALADMINISTRATIONDATASTORE_HPP
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class LocalAdministrationDataStore
      \brief Stores Nexus account info locally.
   */
  class LocalAdministrationDataStore : public AdministrationDataStore {
    public:

      //! Constructs an empty LocalAdministrationDataStore.
      LocalAdministrationDataStore() = default;

      virtual ~LocalAdministrationDataStore() override;

      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        AccountIdentity>> LoadAllAccountIdentities() override;

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters>> LoadAllRiskParameters() override;

      virtual RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState>> LoadAllRiskStates() override;

      virtual RiskService::RiskState LoadRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      virtual AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override;

      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        AccountModificationRequest::Id startId, int maxCount) override;

      virtual EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>
        m_identities;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters> m_riskParameters;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState> m_riskStates;
      std::vector<std::unique_ptr<AccountModificationRequest>>
        m_modificationRequests;
      std::unordered_map<AccountModificationRequest::Id,
        AccountModificationRequest*> m_idToModificationRequests;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        std::vector<AccountModificationRequest*>>
        m_accountToModificationRequests;
      std::unordered_map<AccountModificationRequest::Id,
        EntitlementModification> m_idToEntitlementModification;
      Beam::IO::OpenState m_openState;

      template<typename Request>
      std::vector<AccountModificationRequest::Id> Filter(
        const std::vector<Request>& requests,
        AccountModificationRequest::Id startId, int maxCount);
  };

  inline LocalAdministrationDataStore::~LocalAdministrationDataStore() {
    Close();
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      AccountIdentity>> LocalAdministrationDataStore::
      LoadAllAccountIdentities() {
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      AccountIdentity>> identities;
    identities.reserve(m_identities.size());
    std::transform(m_identities.begin(), m_identities.end(),
      std::back_inserter(identities),
      [] (auto& identity) {
        return std::make_tuple(identity.first, identity.second);
      });
    return identities;
  }

  inline AccountIdentity LocalAdministrationDataStore::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto identity = m_identities.find(account);
    if(identity == m_identities.end()) {
      return {};
    }
    return identity->second;
  }

  inline void LocalAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_identities[account] = identity;
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>> LocalAdministrationDataStore::
      LoadAllRiskParameters() {
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>> riskParameters;
    riskParameters.reserve(m_riskParameters.size());
    std::transform(m_riskParameters.begin(), m_riskParameters.end(),
      std::back_inserter(riskParameters),
      [] (auto& riskParameters) {
        return std::make_tuple(riskParameters.first, riskParameters.second);
      });
    return riskParameters;
  }

  inline RiskService::RiskParameters LocalAdministrationDataStore::
      LoadRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto riskParameters = m_riskParameters.find(account);
    if(riskParameters == m_riskParameters.end()) {
      return {};
    }
    return riskParameters->second;
  }

  inline void LocalAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_riskParameters[account] = riskParameters;
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> LocalAdministrationDataStore::
      LoadAllRiskStates() {
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> riskStates;
    riskStates.reserve(m_riskStates.size());
    std::transform(m_riskStates.begin(), m_riskStates.end(),
      std::back_inserter(riskStates),
      [] (auto& riskState) {
        return std::make_tuple(riskState.first, riskState.second);
      });
    return riskStates;
  }

  inline RiskService::RiskState LocalAdministrationDataStore::LoadRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto riskState = m_riskStates.find(account);
    if(riskState == m_riskStates.end()) {
      return {};
    }
    return riskState->second;
  }

  inline void LocalAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_riskStates[account] = riskState;
  }

  inline AccountModificationRequest
      LocalAdministrationDataStore::LoadAccountModificationRequest(
      AccountModificationRequest::Id id) {
    auto request = m_idToModificationRequests.find(id);
    if(request == m_idToModificationRequests.end()) {
      return {};
    }
    return *request->second;
  }

  inline std::vector<AccountModificationRequest::Id>
      LocalAdministrationDataStore::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    auto requestsIterator = m_accountToModificationRequests.find(account);
    if(requestsIterator == m_accountToModificationRequests.end()) {
      return {};
    }
    auto& requests = requestsIterator->second;
    return Filter(requests, startId, maxCount);
  }

  inline std::vector<AccountModificationRequest::Id>
      LocalAdministrationDataStore::LoadAccountModificationRequestIds(
      AccountModificationRequest::Id startId, int maxCount) {
    return Filter(m_modificationRequests, startId, maxCount);
  }

  inline EntitlementModification LocalAdministrationDataStore::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    auto request = m_idToEntitlementModification.find(id);
    if(request == m_idToEntitlementModification.end()) {
      return {};
    }
    return request->second;
  }

  inline void LocalAdministrationDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    transaction();
  }

  inline void LocalAdministrationDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalAdministrationDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }

  template<typename Request>
  std::vector<AccountModificationRequest::Id>
      LocalAdministrationDataStore::Filter(const std::vector<Request>& requests,
      AccountModificationRequest::Id startId, int maxCount) {
    if(startId == -1) {
      startId = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    auto lastRequest = std::lower_bound(requests.begin(), requests.end(),
      startId,
      [] (auto& lhs, auto rhs) {
        return lhs->GetId() < rhs;
      });
    if(lastRequest != requests.end() && (*lastRequest)->GetId() == startId) {
      if(lastRequest == requests.begin()) {
        return {};
      }
      --lastRequest;
    }
    std::vector<AccountModificationRequest::Id> ids;
    while(static_cast<int>(ids.size()) < maxCount &&
        lastRequest != requests.begin()) {
      ids.push_back((*lastRequest)->GetId());
      --lastRequest;
    }
    if(static_cast<int>(ids.size()) == maxCount) {
      return ids;
    }
    ids.push_back((*lastRequest)->GetId());
    return ids;
  }
}
}

#endif
