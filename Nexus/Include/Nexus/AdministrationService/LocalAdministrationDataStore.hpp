#ifndef NEXUS_LOCAL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_LOCAL_ADMINISTRATION_DATA_STORE_HPP
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/AdministrationService/AccountModificationRequestEvaluatorTranslator.hpp"
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
      std::vector<AccountModificationRequest>
        load_account_modification_requests(
          const std::vector<Beam::DirectoryEntry>& accounts,
          const AccountModificationRequestQuery& query);
      std::vector<AccountModificationRequest>
        load_account_modification_requests(
          const AccountModificationRequestQuery& query);
      AccountModificationRequestCounts load_account_modification_request_counts(
        const std::vector<Beam::DirectoryEntry>& accounts,
        const AccountModificationRequestQuery& query);
      AccountModificationRequestCounts load_account_modification_request_counts(
        const AccountModificationRequestQuery& query);
      std::vector<AccountModificationRequest::Update>
        load_account_modification_request_statuses(
          const std::vector<AccountModificationRequest::Id>& ids);
      std::vector<int> load_message_counts(
        const std::vector<AccountModificationRequest::Id>& ids);
      std::vector<boost::optional<AccountModificationRequest::Id>>
        load_previous_granted_requests(
          const std::vector<AccountModificationRequest::Id>& ids);
      std::vector<EntitlementModification> load_entitlement_modifications(
        const std::vector<AccountModificationRequest::Id>& ids);
      std::vector<RiskModification> load_risk_modifications(
        const std::vector<AccountModificationRequest::Id>& ids);
      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);
      void store_effective_date(AccountModificationRequest::Id id,
        boost::posix_time::ptime effective_date);
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
      std::vector<AccountModificationRequest::Update>
        load_account_modification_request_updates(
          AccountModificationRequest::Id id);
      void store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status);
      Message::Id load_last_message_id();
      Message load_message(Message::Id id);
      std::vector<Message::Id> load_message_ids(
        AccountModificationRequest::Id id);
      void store(const Notification& notification);
      void mark_notification_as_read(const Notification::Id& id);
      void mark_notification_as_unread(const Notification::Id& id);
      std::vector<Notification> load_notifications(
        const Beam::DirectoryEntry& account, const Notification::Id& id,
        Beam::SnapshotLimit limit, Notification::ReadState read_state);
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
      std::unordered_map<Beam::DirectoryEntry, std::vector<Notification>>
        m_notifications;

      static std::unordered_set<Beam::DirectoryEntry> make_account_scope(
        const std::vector<Beam::DirectoryEntry>* accounts);
      LocalAdministrationDataStore(
        const LocalAdministrationDataStore&) = delete;
      LocalAdministrationDataStore& operator =(
        const LocalAdministrationDataStore&) = delete;
      boost::posix_time::ptime load_last_update_timestamp(
        AccountModificationRequest::Id id);
      bool matches_query(const AccountModificationRequest& request,
        const AccountModificationRequestQuery& query);
      AccountModificationRequestCounts count_requests(
        const std::vector<Beam::DirectoryEntry>* accounts,
        const AccountModificationRequestQuery& query);
      std::vector<AccountModificationRequest> load_requests(
        const std::vector<Beam::DirectoryEntry>* accounts,
        const AccountModificationRequestQuery& query);
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

  inline std::vector<AccountModificationRequest>
      LocalAdministrationDataStore::load_account_modification_requests(
        const std::vector<Beam::DirectoryEntry>& accounts,
        const AccountModificationRequestQuery& query) {
    return load_requests(&accounts, query);
  }

  inline std::vector<AccountModificationRequest>
      LocalAdministrationDataStore::load_account_modification_requests(
        const AccountModificationRequestQuery& query) {
    return load_requests(nullptr, query);
  }

  inline AccountModificationRequestCounts
      LocalAdministrationDataStore::load_account_modification_request_counts(
        const std::vector<Beam::DirectoryEntry>& accounts,
        const AccountModificationRequestQuery& query) {
    return count_requests(&accounts, query);
  }

  inline AccountModificationRequestCounts
      LocalAdministrationDataStore::load_account_modification_request_counts(
        const AccountModificationRequestQuery& query) {
    return count_requests(nullptr, query);
  }

  inline boost::posix_time::ptime
      LocalAdministrationDataStore::load_last_update_timestamp(
        AccountModificationRequest::Id id) {
    auto updates = m_account_modification_request_updates.find(id);
    if(updates != m_account_modification_request_updates.end() &&
        !updates->second.empty()) {
      return updates->second.back().m_timestamp;
    }
    auto request = m_account_modification_requests.find(id);
    if(request == m_account_modification_requests.end()) {
      return boost::posix_time::not_a_date_time;
    }
    return request->second.get_timestamp();
  }

  inline bool LocalAdministrationDataStore::matches_query(
      const AccountModificationRequest& request,
      const AccountModificationRequestQuery& query) {
    if(!query.get_categories().empty() &&
        !std::ranges::contains(query.get_categories(), request.get_type())) {
      return false;
    }
    if(!query.get_statuses().empty()) {
      auto status = load_account_modification_request_status(request.get_id());
      if(!std::ranges::contains(query.get_statuses(), status.m_status)) {
        return false;
      }
    }
    auto& start_date = query.get_start_date();
    auto& end_date = query.get_end_date();
    if(start_date || end_date) {
      auto timestamp = load_last_update_timestamp(request.get_id());
      if(start_date && timestamp < *start_date) {
        return false;
      }
      if(end_date && timestamp > *end_date) {
        return false;
      }
    }
    if(auto& excluded_account = query.get_excluded_account()) {
      if(request.get_account().m_id == excluded_account->m_id) {
        return false;
      }
    }
    return true;
  }

  inline std::unordered_set<Beam::DirectoryEntry>
      LocalAdministrationDataStore::make_account_scope(
        const std::vector<Beam::DirectoryEntry>* accounts) {
    if(!accounts) {
      return {};
    }
    return std::unordered_set(accounts->begin(), accounts->end());
  }

  inline AccountModificationRequestCounts
      LocalAdministrationDataStore::count_requests(
        const std::vector<Beam::DirectoryEntry>* accounts,
        const AccountModificationRequestQuery& query) {
    auto evaluator = Beam::translate<
      AccountModificationRequestEvaluatorTranslator>(query.get_filter());
    auto scope = make_account_scope(accounts);
    auto counts = AccountModificationRequestCounts(0, 0, 0);
    for(auto& entry : m_account_modification_requests) {
      auto& request = entry.second;
      if(accounts && !scope.contains(request.get_account())) {
        continue;
      }
      if(!Beam::test_filter(*evaluator, request)) {
        continue;
      }
      if(!matches_query(request, query)) {
        continue;
      }
      auto status =
        load_account_modification_request_status(request.get_id()).m_status;
      if(status == AccountModificationRequest::Status::GRANTED) {
        ++counts.m_granted;
      } else if(status == AccountModificationRequest::Status::REJECTED) {
        ++counts.m_rejected;
      } else {
        ++counts.m_pending;
      }
    }
    return counts;
  }

  inline std::vector<AccountModificationRequest>
      LocalAdministrationDataStore::load_requests(
        const std::vector<Beam::DirectoryEntry>* accounts,
        const AccountModificationRequestQuery& query) {
    auto evaluator = Beam::translate<
      AccountModificationRequestEvaluatorTranslator>(query.get_filter());
    auto& anchor = query.get_anchor();
    auto is_head =
      query.get_snapshot_limit().get_type() == Beam::SnapshotLimit::Type::HEAD;
    auto field = query.get_sort_field();
    using SortKey =
      std::pair<boost::posix_time::ptime, AccountModificationRequest::Id>;
    auto key = [&] (const AccountModificationRequest& request) {
      if(field == AccountModificationRequestQuery::SortField::LAST_UPDATED) {
        return SortKey(
          load_last_update_timestamp(request.get_id()), request.get_id());
      } else if(
          field == AccountModificationRequestQuery::SortField::EFFECTIVE_DATE) {
        return SortKey(request.get_effective_date(), request.get_id());
      }
      return SortKey(boost::posix_time::neg_infin, request.get_id());
    };
    auto anchor_key = [&] {
      if(!anchor) {
        return SortKey();
      }
      if(field == AccountModificationRequestQuery::SortField::LAST_UPDATED ||
          field == AccountModificationRequestQuery::SortField::EFFECTIVE_DATE) {
        return SortKey(anchor->m_date, anchor->m_id);
      }
      return SortKey(boost::posix_time::neg_infin, anchor->m_id);
    }();
    auto scope = make_account_scope(accounts);
    auto matches = std::vector<AccountModificationRequest>();
    for(auto& entry : m_account_modification_requests) {
      auto& request = entry.second;
      if(accounts && !scope.contains(request.get_account())) {
        continue;
      }
      auto is_excluded = [&] {
        if(!anchor) {
          return false;
        }
        if(is_head) {
          return key(request) <= anchor_key;
        }
        return key(request) >= anchor_key;
      }();
      if(is_excluded) {
        continue;
      }
      if(!Beam::test_filter(*evaluator, request)) {
        continue;
      }
      if(!matches_query(request, query)) {
        continue;
      }
      matches.push_back(request);
    }
    std::ranges::sort(matches, std::ranges::less(), key);
    auto offset = query.get_offset();
    auto skip = std::min<std::size_t>(offset, matches.size());
    if(is_head) {
      matches.erase(matches.begin(), matches.begin() + skip);
    } else {
      matches.erase(matches.end() - skip, matches.end());
    }
    auto size = std::max(0, query.get_snapshot_limit().get_size());
    if(matches.size() > static_cast<std::size_t>(size)) {
      if(is_head) {
        matches.erase(matches.begin() + size, matches.end());
      } else {
        matches.erase(matches.begin(), matches.end() - size);
      }
    }
    return matches;
  }

  inline std::vector<AccountModificationRequest::Update>
      LocalAdministrationDataStore::load_account_modification_request_statuses(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto statuses = std::vector<AccountModificationRequest::Update>();
    statuses.reserve(ids.size());
    for(auto id : ids) {
      statuses.push_back(load_account_modification_request_status(id));
    }
    return statuses;
  }

  inline std::vector<int> LocalAdministrationDataStore::load_message_counts(
      const std::vector<AccountModificationRequest::Id>& ids) {
    auto counts = std::vector<int>();
    counts.reserve(ids.size());
    for(auto id : ids) {
      auto i = m_request_messages.find(id);
      if(i == m_request_messages.end()) {
        counts.push_back(0);
      } else {
        counts.push_back(static_cast<int>(i->second.size()));
      }
    }
    return counts;
  }

  inline std::vector<boost::optional<AccountModificationRequest::Id>>
      LocalAdministrationDataStore::load_previous_granted_requests(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto predecessors =
      std::vector<boost::optional<AccountModificationRequest::Id>>();
    predecessors.reserve(ids.size());
    for(auto id : ids) {
      auto request = m_account_modification_requests.find(id);
      if(request == m_account_modification_requests.end()) {
        predecessors.emplace_back();
        continue;
      }
      auto predecessor = boost::optional<AccountModificationRequest::Id>();
      for(auto& entry : m_account_modification_requests) {
        auto& candidate = entry.second;
        if(candidate.get_id() >= id ||
            candidate.get_account() != request->second.get_account() ||
            candidate.get_type() != request->second.get_type()) {
          continue;
        }
        if(load_account_modification_request_status(
            candidate.get_id()).m_status !=
              AccountModificationRequest::Status::GRANTED) {
          continue;
        }
        if(!predecessor || *predecessor < candidate.get_id()) {
          predecessor = candidate.get_id();
        }
      }
      predecessors.push_back(predecessor);
    }
    return predecessors;
  }

  inline std::vector<EntitlementModification>
      LocalAdministrationDataStore::load_entitlement_modifications(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto modifications = std::vector<EntitlementModification>();
    modifications.reserve(ids.size());
    for(auto id : ids) {
      modifications.push_back(load_entitlement_modification(id));
    }
    return modifications;
  }

  inline std::vector<RiskModification>
      LocalAdministrationDataStore::load_risk_modifications(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto modifications = std::vector<RiskModification>();
    modifications.reserve(ids.size());
    for(auto id : ids) {
      modifications.push_back(load_risk_modification(id));
    }
    return modifications;
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

  inline void LocalAdministrationDataStore::store_effective_date(
      AccountModificationRequest::Id id,
      boost::posix_time::ptime effective_date) {
    auto i = m_account_modification_requests.find(id);
    if(i == m_account_modification_requests.end()) {
      return;
    }
    auto& request = i->second;
    request = AccountModificationRequest(request.get_id(), request.get_type(),
      request.get_account(), request.get_submission_account(),
      request.get_timestamp(), effective_date);
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

  inline std::vector<AccountModificationRequest::Update>
      LocalAdministrationDataStore::load_account_modification_request_updates(
        AccountModificationRequest::Id id) {
    auto i = m_account_modification_request_updates.find(id);
    if(i == m_account_modification_request_updates.end()) {
      return {};
    }
    return i->second;
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

  inline void LocalAdministrationDataStore::store(
      const Notification& notification) {
    m_notifications[notification.m_account].push_back(notification);
  }

  inline void LocalAdministrationDataStore::mark_notification_as_read(
      const Notification::Id& id) {
    auto all = m_notifications | std::views::values | std::views::join;
    auto i = std::ranges::find(all, id, &Notification::m_id);
    if(i != all.end()) {
      i->m_is_read = true;
    }
  }

  inline void LocalAdministrationDataStore::mark_notification_as_unread(
      const Notification::Id& id) {
    auto all = m_notifications | std::views::values | std::views::join;
    auto i = std::ranges::find(all, id, &Notification::m_id);
    if(i != all.end()) {
      i->m_is_read = false;
    }
  }

  inline std::vector<Notification>
      LocalAdministrationDataStore::load_notifications(
        const Beam::DirectoryEntry& account, const Notification::Id& id,
        Beam::SnapshotLimit limit, Notification::ReadState read_state) {
    auto i = m_notifications.find(account);
    if(i == m_notifications.end()) {
      return {};
    }
    auto& notifications = i->second;
    auto matches = std::vector<Notification>();
    for(auto& notification : notifications) {
      if(read_state == Notification::ReadState::UNREAD &&
          notification.m_is_read ||
          read_state == Notification::ReadState::READ &&
            !notification.m_is_read) {
        continue;
      }
      matches.push_back(notification);
    }
    if(id.empty()) {
      if(limit.get_type() == Beam::SnapshotLimit::Type::TAIL) {
        if(static_cast<int>(matches.size()) > limit.get_size()) {
          matches.erase(
            matches.begin(), matches.end() - limit.get_size());
        }
      } else if(static_cast<int>(matches.size()) > limit.get_size()) {
        matches.erase(matches.begin() + limit.get_size(), matches.end());
      }
      return matches;
    }
    auto position = std::ranges::find_if(matches, [&] (const auto& n) {
      return n.m_id == id;
    });
    if(position == matches.end()) {
      return {};
    }
    if(limit.get_type() == Beam::SnapshotLimit::Type::TAIL) {
      auto start = position - std::min(
        static_cast<int>(position - matches.begin()), limit.get_size() - 1);
      return std::vector(start, position + 1);
    }
    auto end = position + std::min(
      static_cast<int>(matches.end() - position), limit.get_size());
    return std::vector(position, end);
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
