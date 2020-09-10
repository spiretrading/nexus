#ifndef NEXUS_LOCAL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_LOCAL_ADMINISTRATION_DATA_STORE_HPP
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus::AdministrationService {

  /** Stores Nexus account info locally. */
  class LocalAdministrationDataStore : public AdministrationDataStore {
    public:

      /** Constructs an empty LocalAdministrationDataStore. */
      LocalAdministrationDataStore();

      ~LocalAdministrationDataStore() override;

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

      AccountModificationRequest::Update
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      void Store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status) override;

      Message::Id LoadLastMessageId() override;

      Message LoadMessage(Message::Id id) override;

      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Close() override;

      using AdministrationDataStore::WithTransaction;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>
        m_identities;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters> m_riskParameters;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState> m_riskStates;
      std::unordered_map<AccountModificationRequest::Id,
        AccountModificationRequest> m_idToModificationRequests;
      std::vector<AccountModificationRequest::Id> m_modificationRequests;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        std::vector<AccountModificationRequest::Id>>
        m_accountToModificationRequests;
      std::unordered_map<AccountModificationRequest::Id,
        EntitlementModification> m_idToEntitlementModification;
      std::unordered_map<AccountModificationRequest::Id,
        RiskModification> m_idToRiskModification;
      std::unordered_map<Message::Id, Message> m_idToMessage;
      std::unordered_map<AccountModificationRequest::Id,
        std::vector<Message::Id>> m_requestToMessages;
      std::unordered_map<AccountModificationRequest::Id,
        AccountModificationRequest::Update> m_idToRequestStatus;
      Message::Id m_lastMessageId;
      Beam::IO::OpenState m_openState;

      std::vector<AccountModificationRequest::Id> Filter(
        const std::vector<AccountModificationRequest::Id>& requests,
        AccountModificationRequest::Id startId, int maxCount);
      void Store(const AccountModificationRequest& request);
  };

  inline LocalAdministrationDataStore::LocalAdministrationDataStore()
    : m_lastMessageId(0) {}

  inline LocalAdministrationDataStore::~LocalAdministrationDataStore() {
    Close();
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      AccountIdentity>> LocalAdministrationDataStore::
      LoadAllAccountIdentities() {
    auto identities = std::vector<
      std::tuple<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>>();
    identities.reserve(m_identities.size());
    std::transform(m_identities.begin(), m_identities.end(),
      std::back_inserter(identities),
      [] (auto& identity) {
        return std::tuple(identity.first, identity.second);
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
    auto riskParameters = std::vector<
      std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>>();
    riskParameters.reserve(m_riskParameters.size());
    std::transform(m_riskParameters.begin(), m_riskParameters.end(),
      std::back_inserter(riskParameters),
      [] (auto& riskParameters) {
        return std::tuple(riskParameters.first, riskParameters.second);
      });
    return riskParameters;
  }

  inline RiskService::RiskParameters LocalAdministrationDataStore::
      LoadRiskParameters(const Beam::ServiceLocator::DirectoryEntry& account) {
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
    auto riskStates = std::vector<std::tuple<
      Beam::ServiceLocator::DirectoryEntry, RiskService::RiskState>>();
    riskStates.reserve(m_riskStates.size());
    std::transform(m_riskStates.begin(), m_riskStates.end(),
      std::back_inserter(riskStates),
      [] (auto& riskState) {
        return std::tuple(riskState.first, riskState.second);
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
    return request->second;
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

  inline void LocalAdministrationDataStore::Store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    Store(request);
    m_idToEntitlementModification.insert(
      std::pair(request.GetId(), modification));
  }

  inline RiskModification LocalAdministrationDataStore::LoadRiskModification(
      AccountModificationRequest::Id id) {
    auto request = m_idToRiskModification.find(id);
    if(request == m_idToRiskModification.end()) {
      return {};
    }
    return request->second;
  }

  inline void LocalAdministrationDataStore::Store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    Store(request);
    m_idToRiskModification.insert(std::pair(request.GetId(), modification));
  }

  inline void LocalAdministrationDataStore::Store(
      AccountModificationRequest::Id id, const Message& message) {
    m_idToMessage.insert(std::pair(message.GetId(), message));
    m_requestToMessages[id].push_back(message.GetId());
    m_lastMessageId = message.GetId();
  }

  inline AccountModificationRequest::Update
      LocalAdministrationDataStore::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    auto status = m_idToRequestStatus.find(id);
    if(status == m_idToRequestStatus.end()) {
      return {};
    }
    return status->second;
  }

  inline void LocalAdministrationDataStore::Store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    m_idToRequestStatus[id] = status;
  }

  inline Message::Id LocalAdministrationDataStore::LoadLastMessageId() {
    return m_lastMessageId;
  }

  inline Message LocalAdministrationDataStore::LoadMessage(Message::Id id) {
    auto message = m_idToMessage.find(id);
    if(message == m_idToMessage.end()) {
      return {};
    }
    return message->second;
  }

  inline std::vector<Message::Id> LocalAdministrationDataStore::LoadMessageIds(
      AccountModificationRequest::Id id) {
    auto messageIds = m_requestToMessages.find(id);
    if(messageIds == m_requestToMessages.end()) {
      return {};
    }
    return messageIds->second;
  }

  inline void LocalAdministrationDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    transaction();
  }

  inline void LocalAdministrationDataStore::Close() {
    m_openState.Close();
  }

  inline std::vector<AccountModificationRequest::Id>
      LocalAdministrationDataStore::Filter(
      const std::vector<AccountModificationRequest::Id>& requests,
      AccountModificationRequest::Id startId, int maxCount) {
    if(requests.empty()) {
      return {};
    }
    if(startId == -1) {
      startId = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    auto lastRequest = std::lower_bound(requests.begin(), requests.end(),
      startId);
    if(lastRequest != requests.end() && *lastRequest == startId) {
      if(lastRequest == requests.begin()) {
        return {};
      }
      --lastRequest;
    }
    auto ids = std::vector<AccountModificationRequest::Id>();
    while(static_cast<int>(ids.size()) < maxCount &&
        lastRequest != requests.begin()) {
      ids.push_back(*lastRequest);
      --lastRequest;
    }
    if(static_cast<int>(ids.size()) == maxCount) {
      return ids;
    }
    ids.push_back(*lastRequest);
    return ids;
  }

  inline void LocalAdministrationDataStore::Store(
      const AccountModificationRequest& request) {
    m_idToModificationRequests.insert(std::pair(request.GetId(), request));
    if(m_modificationRequests.empty() ||
        request.GetId() > m_modificationRequests.back()) {
      m_modificationRequests.push_back(request.GetId());
      m_accountToModificationRequests[request.GetAccount()].push_back(
        request.GetId());
    } else {
      auto insertionPoint = std::lower_bound(m_modificationRequests.begin(),
        m_modificationRequests.end(), request.GetId());
      m_modificationRequests.insert(insertionPoint,
        request.GetId());
      auto& accountRequests = m_accountToModificationRequests[
        request.GetAccount()];
      auto accountInsertion = std::lower_bound(accountRequests.begin(),
        accountRequests.end(), request.GetId());
      accountRequests.insert(accountInsertion, request.GetId());
    }
  }
}

#endif
