#ifndef NEXUS_LOCAL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_LOCAL_ADMINISTRATION_DATA_STORE_HPP
#include <unordered_map>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"

namespace Nexus {

  /**
   * Implements the AdministrationDataStore interface using in-memory data
   * structures.
   */
  class LocalAdministrationDataStore {
    public:
      using IndexedAccountIdentity =
        AdministrationDataStore::IndexedAccountIdentity;
      using IndexedRiskParameters =
        AdministrationDataStore::IndexedRiskParameters;
      using IndexedRiskState = AdministrationDataStore::IndexedRiskState;

      /** Constructs an empty LocalAdministrationDataStore. */
      LocalAdministrationDataStore() = default;

      std::vector<IndexedAccountIdentity> load_all_account_identities();
      AccountIdentity load_identity(
        const Beam::DirectoryEntry& account);
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
      mutable boost::mutex m_mutex;
      std::unordered_map<Beam::DirectoryEntry, AccountIdentity>
        m_account_identities;
      std::unordered_map<Beam::DirectoryEntry, RiskParameters>
        m_risk_parameters;
      std::unordered_map<Beam::DirectoryEntry, RiskState> m_risk_states;
      std::unordered_map<AccountModificationRequest::Id,
        AccountModificationRequest> m_account_modification_requests;
      std::unordered_map<AccountModificationRequest::Id,
        EntitlementModification> m_entitlement_modifications;
      std::unordered_map<AccountModificationRequest::Id, RiskModification>
        m_risk_modifications;
      std::unordered_map<AccountModificationRequest::Id,
        std::vector<AccountModificationRequest::Update>>
          m_account_modification_request_updates;
      std::unordered_map<AccountModificationRequest::Id,
        std::vector<Message::Id>> m_request_messages;
      std::unordered_map<Message::Id, Message> m_messages;
      Message::Id m_last_message_id;

      LocalAdministrationDataStore(
        const LocalAdministrationDataStore&) = delete;
      LocalAdministrationDataStore& operator =(
        const LocalAdministrationDataStore&) = delete;
  };

  inline std::vector<LocalAdministrationDataStore::IndexedAccountIdentity>
      LocalAdministrationDataStore::load_all_account_identities() {
    auto result = std::vector<IndexedAccountIdentity>();
    result.reserve(m_account_identities.size());
    for(auto& entry : m_account_identities) {
      result.push_back({entry.first, entry.second});
    }
    return result;
  }

  inline AccountIdentity LocalAdministrationDataStore::load_identity(
      const Beam::DirectoryEntry& account) {
    auto i = m_account_identities.find(account);
    if(i == m_account_identities.end()) {
      return AccountIdentity();
    }
    return i->second;
  }

  inline void LocalAdministrationDataStore::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    m_account_identities[account] = identity;
  }

  inline std::vector<LocalAdministrationDataStore::IndexedRiskParameters>
      LocalAdministrationDataStore::load_all_risk_parameters() {
    auto result = std::vector<IndexedRiskParameters>();
    result.reserve(m_risk_parameters.size());
    for(auto& entry : m_risk_parameters) {
      result.push_back({entry.first, entry.second});
    }
    return result;
  }

  inline RiskParameters LocalAdministrationDataStore::load_risk_parameters(
      const Beam::DirectoryEntry& account) {
    auto i = m_risk_parameters.find(account);
    if(i == m_risk_parameters.end()) {
      return RiskParameters();
    }
    return i->second;
  }

  inline void LocalAdministrationDataStore::store(
      const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    m_risk_parameters[account] = risk_parameters;
  }

  inline std::vector<LocalAdministrationDataStore::IndexedRiskState>
      LocalAdministrationDataStore::load_all_risk_states() {
    auto result = std::vector<IndexedRiskState>();
    result.reserve(m_risk_states.size());
    for(auto& entry : m_risk_states) {
      result.push_back({entry.first, entry.second});
    }
    return result;
  }

  inline RiskState LocalAdministrationDataStore::load_risk_state(
      const Beam::DirectoryEntry& account) {
    auto i = m_risk_states.find(account);
    if(i == m_risk_states.end()) {
      return RiskState();
    }
    return i->second;
  }

  inline void LocalAdministrationDataStore::store(
      const Beam::DirectoryEntry& account, const RiskState& risk_state) {
    m_risk_states[account] = risk_state;
  }

  inline AccountModificationRequest
      LocalAdministrationDataStore::load_account_modification_request(
        AccountModificationRequest::Id id) {
    auto i = m_account_modification_requests.find(id);
    if(i == m_account_modification_requests.end()) {
      return AccountModificationRequest();
    }
    return i->second;
  }

  inline std::vector<AccountModificationRequest::Id>
      LocalAdministrationDataStore::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    auto ids = std::vector<AccountModificationRequest::Id>();
    auto i = m_account_modification_requests.begin();
    while(i != m_account_modification_requests.end()) {
      if(i->second.get_id() > start_id && i->second.get_account() == account) {
        ids.push_back(i->first);
      }
      ++i;
    }
    std::sort(ids.begin(), ids.end());
    while(ids.size() > max_count) {
      ids.pop_back();
    }
    return ids;
  }

  inline std::vector<AccountModificationRequest::Id>
      LocalAdministrationDataStore::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    auto ids = std::vector<AccountModificationRequest::Id>();
    auto i = m_account_modification_requests.begin();
    while(i != m_account_modification_requests.end()) {
      if(i->second.get_id() > start_id) {
        ids.push_back(i->first);
      }
      ++i;
    }
    std::sort(ids.begin(), ids.end());
    while(ids.size() > max_count) {
      ids.pop_back();
    }
    return ids;
  }

  inline EntitlementModification
      LocalAdministrationDataStore::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    auto i = m_entitlement_modifications.find(id);
    if(i == m_entitlement_modifications.end()) {
      return EntitlementModification();
    }
    return i->second;
  }

  inline void LocalAdministrationDataStore::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    m_account_modification_requests[request.get_id()] = request;
    m_entitlement_modifications[request.get_id()] = modification;
  }

  inline RiskModification LocalAdministrationDataStore::load_risk_modification(
      AccountModificationRequest::Id id) {
    auto i = m_risk_modifications.find(id);
    if(i == m_risk_modifications.end()) {
      return RiskModification();
    }
    return i->second;
  }

  inline void LocalAdministrationDataStore::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    m_account_modification_requests[request.get_id()] = request;
    m_risk_modifications[request.get_id()] = modification;
  }

  inline void LocalAdministrationDataStore::store(
      AccountModificationRequest::Id id, const Message& message) {
    m_messages[message.get_id()] = message;
    m_request_messages[id].push_back(message.get_id());
    if(m_messages.size() == 1) {
      m_last_message_id = message.get_id();
    } else {
      m_last_message_id = std::max(m_last_message_id, message.get_id());
    }
  }

  inline AccountModificationRequest::Update
      LocalAdministrationDataStore::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    auto i = m_account_modification_request_updates.find(id);
    if(i == m_account_modification_request_updates.end() || i->second.empty()) {
      return AccountModificationRequest::Update();
    }
    return i->second.back();
  }

  inline void LocalAdministrationDataStore::store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    m_account_modification_request_updates[id].push_back(status);
  }

  inline Message::Id LocalAdministrationDataStore::load_last_message_id() {
    if(m_messages.empty()) {
      return 0;
    }
    return m_last_message_id;
  }

  inline Message LocalAdministrationDataStore::load_message(Message::Id id) {
    auto i = m_messages.find(id);
    if(i == m_messages.end()) {
      return Message();
    }
    return i->second;
  }

  inline std::vector<Message::Id>
      LocalAdministrationDataStore::load_message_ids(
        AccountModificationRequest::Id id) {
    auto i = m_request_messages.find(id);
    if(i == m_request_messages.end()) {
      return std::vector<Message::Id>();
    }
    return i->second;
  }

  template<typename F>
  decltype(auto) LocalAdministrationDataStore::with_transaction(
      F&& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    return std::forward<F>(transaction)();
  }

  inline void LocalAdministrationDataStore::close() {}
}

#endif
