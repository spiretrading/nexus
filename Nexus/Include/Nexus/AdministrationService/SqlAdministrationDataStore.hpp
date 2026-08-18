#ifndef NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#include <algorithm>
#include <map>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/AdministrationService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus {

  /**
   * Stores account administration data in a MySQL database.
   * @param <C> The type of SQL connection.
   */
  template<typename C>
  class SqlAdministrationDataStore {
    public:
      using IndexedAccountIdentity =
        AdministrationDataStore::IndexedAccountIdentity;
      using IndexedRiskParameters =
        AdministrationDataStore::IndexedRiskParameters;
      using IndexedRiskState = AdministrationDataStore::IndexedRiskState;

      /** The type of SQL connection. */
      using Connection = C;

      /** The function used to load DirectoryEntries. */
      using DirectoryEntrySource = Beam::KeyValueCache<
        unsigned int, Beam::DirectoryEntry, Beam::Mutex>::Source;

      /**
       * Constructs an SqlAdministrationDataStore.
       * @param connection The connection to the SQL database.
       * @param directory_entry_source The function used to load
       *        DirectoryEntries.
       */
      SqlAdministrationDataStore(std::unique_ptr<Connection> connection,
        DirectoryEntrySource directory_entry_source);

      ~SqlAdministrationDataStore();

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
      mutable Beam::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::KeyValueCache<unsigned int, Beam::DirectoryEntry, Beam::Mutex>
        m_directory_entries;
      Beam::OpenState m_open_state;

      static Viper::Expression make_id_filter(const std::string& column,
        const std::vector<AccountModificationRequest::Id>& ids);
      static Viper::Expression make_account_filter(
        const std::vector<Beam::DirectoryEntry>& accounts);
      static std::string get_sort_column(
        AccountModificationRequestQuery::SortField field);
      static Viper::Expression make_query_filter(
        const AccountModificationRequestQuery& query);
      std::vector<AccountModificationRequest> load_requests(
        const Viper::Expression& accounts,
        const AccountModificationRequestQuery& query);
      AccountModificationRequestCounts count_requests(
        const Viper::Expression& accounts,
        const AccountModificationRequestQuery& query);
  };

  template<typename C>
  SqlAdministrationDataStore<C>::SqlAdministrationDataStore(
      std::unique_ptr<Connection> connection,
      DirectoryEntrySource directory_entry_source)
      : m_connection(std::move(connection)),
        m_directory_entries(std::move(directory_entry_source)) {
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        get_indexed_account_identity_row(), "account_identities"));
      m_connection->execute(Viper::create_if_not_exists(
        get_indexed_risk_parameters_row(), "risk_parameters"));
      m_connection->execute(Viper::create_if_not_exists(
        get_indexed_risk_state_row(), "risk_states"));
      m_connection->execute(Viper::create_if_not_exists(
        get_stored_account_modification_request_row(),
        "account_modification_requests"));
      m_connection->execute(Viper::create_if_not_exists(
        get_entitlement_modification_row(), "entitlement_modifications"));
      m_connection->execute(Viper::create_if_not_exists(
        get_risk_modification_row(), "risk_modifications"));
      m_connection->execute(Viper::create_if_not_exists(
        get_indexed_account_modification_request_status(),
        "account_modification_request_status"));
      m_connection->execute(Viper::create_if_not_exists(
        get_administration_message_index_row(), "administration_messages"));
      m_connection->execute(Viper::create_if_not_exists(
        get_indexed_message_body_row(), "administration_message_bodies"));
      m_connection->execute(Viper::create_if_not_exists(
        get_account_modification_request_message_index_row(),
        "account_modification_request_messages"));
      m_connection->execute(Viper::create_if_not_exists(
        get_notification_row(), "notifications"));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlAdministrationDataStore<C>::~SqlAdministrationDataStore() {
    close();
  }

  template<typename C>
  std::vector<AdministrationDataStore::IndexedAccountIdentity>
      SqlAdministrationDataStore<C>::load_all_account_identities() {
    auto identities = std::vector<IndexedAccountIdentity>();
    try {
      m_connection->execute(Viper::select(get_indexed_account_identity_row(),
        "account_identities", std::back_inserter(identities)));
      for(auto& identity : identities) {
        identity.m_index = m_directory_entries.load(identity.m_index.m_id);
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return identities;
  }

  template<typename C>
  AccountIdentity SqlAdministrationDataStore<C>::load_identity(
      const Beam::DirectoryEntry& account) {
    auto identity = AccountIdentity();
    try {
      m_connection->execute(Viper::select(get_account_identity_row(),
        "account_identities", Viper::sym("account") == account.m_id,
        &identity));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return identity;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(const Beam::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto row = IndexedAccountIdentity(account, identity);
    try {
      m_connection->execute(Viper::upsert(
        get_indexed_account_identity_row(), "account_identities", &row));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<AdministrationDataStore::IndexedRiskParameters>
      SqlAdministrationDataStore<C>::load_all_risk_parameters() {
    auto parameters = std::vector<IndexedRiskParameters>();
    try {
      m_connection->execute(Viper::select(get_indexed_risk_parameters_row(),
        "risk_parameters", std::back_inserter(parameters)));
      for(auto& parameter : parameters) {
        parameter.m_index = m_directory_entries.load(parameter.m_index.m_id);
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return parameters;
  }

  template<typename C>
  RiskParameters SqlAdministrationDataStore<C>::load_risk_parameters(
      const Beam::DirectoryEntry& account) {
    auto parameters = RiskParameters();
    try {
      m_connection->execute(Viper::select(get_risk_parameters_row(),
        "risk_parameters", Viper::sym("account") == account.m_id, &parameters));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return parameters;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    auto parameters = IndexedRiskParameters(account, risk_parameters);
    try {
      m_connection->execute(Viper::upsert(
        get_indexed_risk_parameters_row(), "risk_parameters", &parameters));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<AdministrationDataStore::IndexedRiskState>
      SqlAdministrationDataStore<C>::load_all_risk_states() {
    auto states = std::vector<IndexedRiskState>();
    try {
      m_connection->execute(Viper::select(get_indexed_risk_state_row(),
        "risk_states", std::back_inserter(states)));
      for(auto& state : states) {
        state.m_index = m_directory_entries.load(state.m_index.m_id);
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return states;
  }

  template<typename C>
  RiskState SqlAdministrationDataStore<C>::load_risk_state(
      const Beam::DirectoryEntry& account) {
    auto state = RiskState();
    try {
      m_connection->execute(Viper::select(get_risk_state_row(),
        "risk_states", Viper::sym("account") == account.m_id, &state));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return state;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(const Beam::DirectoryEntry& account,
      const RiskState& risk_state) {
    auto indexed_state = IndexedRiskState(account, risk_state);
    try {
      m_connection->execute(Viper::upsert(
        get_indexed_risk_state_row(), "risk_states", &indexed_state));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  AccountModificationRequest SqlAdministrationDataStore<C>::
      load_account_modification_request(AccountModificationRequest::Id id) {
    auto request = AccountModificationRequest();
    try {
      m_connection->execute(Viper::select(
        get_account_modification_request_row(), "account_modification_requests",
        Viper::sym("id") == id, &request));
      request = AccountModificationRequest(request.get_id(), request.get_type(),
        m_directory_entries.load(request.get_account().m_id),
        m_directory_entries.load(request.get_submission_account().m_id),
        request.get_timestamp(), request.get_effective_date());
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return request;
  }

  template<typename C>
  std::vector<AccountModificationRequest>
      SqlAdministrationDataStore<C>::load_account_modification_requests(
        const std::vector<Beam::DirectoryEntry>& accounts,
        const AccountModificationRequestQuery& query) {
    auto filter = make_account_filter(accounts);
    return load_requests(filter, query);
  }

  template<typename C>
  std::vector<AccountModificationRequest>
      SqlAdministrationDataStore<C>::load_account_modification_requests(
        const AccountModificationRequestQuery& query) {
    return load_requests(Viper::literal(true), query);
  }

  template<typename C>
  AccountModificationRequestCounts
      SqlAdministrationDataStore<C>::load_account_modification_request_counts(
        const std::vector<Beam::DirectoryEntry>& accounts,
        const AccountModificationRequestQuery& query) {
    auto filter = make_account_filter(accounts);
    return count_requests(filter, query);
  }

  template<typename C>
  AccountModificationRequestCounts
      SqlAdministrationDataStore<C>::load_account_modification_request_counts(
        const AccountModificationRequestQuery& query) {
    return count_requests(Viper::literal(true), query);
  }

  template<typename C>
  Viper::Expression SqlAdministrationDataStore<C>::make_id_filter(
      const std::string& column,
      const std::vector<AccountModificationRequest::Id>& ids) {
    return Viper::in(Viper::sym(column), ids.begin(), ids.end());
  }

  template<typename C>
  Viper::Expression SqlAdministrationDataStore<C>::make_account_filter(
      const std::vector<Beam::DirectoryEntry>& accounts) {
    auto ids = std::vector<Viper::Expression>();
    ids.reserve(accounts.size());
    for(auto& account : accounts) {
      ids.push_back(Viper::literal(account.m_id));
    }
    return Viper::in(Viper::sym("account"), std::move(ids));
  }

  template<typename C>
  std::string SqlAdministrationDataStore<C>::get_sort_column(
      AccountModificationRequestQuery::SortField field) {
    if(field == AccountModificationRequestQuery::SortField::LAST_UPDATED) {
      return "last_update_timestamp";
    } else if(
        field == AccountModificationRequestQuery::SortField::EFFECTIVE_DATE) {
      return "effective_date";
    }
    return "";
  }

  template<typename C>
  Viper::Expression SqlAdministrationDataStore<C>::make_query_filter(
      const AccountModificationRequestQuery& query) {
    auto filter = Viper::literal(true);
    if(!query.get_categories().empty()) {
      filter = filter && Viper::in(Viper::sym("type"),
        query.get_categories().begin(), query.get_categories().end());
    }
    if(!query.get_statuses().empty()) {
      filter = filter && Viper::in(Viper::sym("status"),
        query.get_statuses().begin(), query.get_statuses().end());
    }
    if(auto& start_date = query.get_start_date()) {
      filter = filter && Viper::sym("last_update_timestamp") >= *start_date;
    }
    if(auto& end_date = query.get_end_date()) {
      filter = filter && Viper::sym("last_update_timestamp") <= *end_date;
    }
    if(auto& excluded_account = query.get_excluded_account()) {
      filter = filter && Viper::sym("account") != excluded_account->m_id;
    }
    return filter;
  }

  template<typename C>
  AccountModificationRequestCounts
      SqlAdministrationDataStore<C>::count_requests(
        const Viper::Expression& accounts,
        const AccountModificationRequestQuery& query) {
    auto filter = make_sql_query(
      "account_modification_requests", query.get_filter()) &&
      make_query_filter(query) && accounts;
    auto count = [&] (const Viper::Expression& status) {
      auto result = 0;
      m_connection->execute(Viper::select(Viper::count("id"),
        "account_modification_requests", filter && status, &result));
      return result;
    };
    auto counts = AccountModificationRequestCounts(0, 0, 0);
    try {
      counts.m_granted = count(
        Viper::sym("status") == AccountModificationRequest::Status::GRANTED);
      counts.m_rejected = count(
        Viper::sym("status") == AccountModificationRequest::Status::REJECTED);
      counts.m_pending = count(
        Viper::sym("status") != AccountModificationRequest::Status::GRANTED &&
        Viper::sym("status") != AccountModificationRequest::Status::REJECTED);
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return counts;
  }

  template<typename C>
  std::vector<AccountModificationRequest>
      SqlAdministrationDataStore<C>::load_requests(
        const Viper::Expression& accounts,
        const AccountModificationRequestQuery& query) {
    auto filter = make_sql_query(
      "account_modification_requests", query.get_filter()) &&
      make_query_filter(query);
    auto is_head =
      query.get_snapshot_limit().get_type() == Beam::SnapshotLimit::Type::HEAD;
    auto sort_column = get_sort_column(query.get_sort_field());
    auto anchor = [&] {
      auto anchor = query.get_anchor();
      if(!anchor) {
        return Viper::literal(true);
      }
      if(sort_column.empty()) {
        if(is_head) {
          return Viper::sym("id") > anchor->m_id;
        }
        return Viper::sym("id") < anchor->m_id;
      }
      if(is_head) {
        return Viper::sym(sort_column) > anchor->m_date ||
          (Viper::sym(sort_column) == anchor->m_date &&
            Viper::sym("id") > anchor->m_id);
      }
      return Viper::sym(sort_column) < anchor->m_date ||
        (Viper::sym(sort_column) == anchor->m_date &&
          Viper::sym("id") < anchor->m_id);
    }();
    auto direction = [&] {
      if(is_head) {
        return Viper::Order::ASC;
      }
      return Viper::Order::DESC;
    }();
    auto order = std::vector<Viper::Order::Column>();
    if(!sort_column.empty()) {
      order.emplace_back(sort_column, direction);
    }
    order.emplace_back("id", direction);
    auto requests = std::vector<AccountModificationRequest>();
    try {
      m_connection->execute(Viper::select(
        get_account_modification_request_row(),
        "account_modification_requests", filter && anchor && accounts,
        Viper::order_by(std::move(order)),
        Viper::limit(std::max(0, query.get_snapshot_limit().get_size()),
          query.get_offset()),
        std::back_inserter(requests)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    if(!is_head) {
      std::ranges::reverse(requests);
    }
    for(auto& request : requests) {
      request = AccountModificationRequest(request.get_id(), request.get_type(),
        m_directory_entries.load(request.get_account().m_id),
        m_directory_entries.load(request.get_submission_account().m_id),
        request.get_timestamp(), request.get_effective_date());
    }
    return requests;
  }

  template<typename C>
  std::vector<AccountModificationRequest::Update>
      SqlAdministrationDataStore<C>::load_account_modification_request_statuses(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto statuses = std::vector<AccountModificationRequest::Update>(ids.size());
    if(ids.empty()) {
      return statuses;
    }
    auto rows = std::vector<IndexedAccountModificationRequestStatus>();
    try {
      m_connection->execute(Viper::select(
        get_indexed_account_modification_request_status(),
        "account_modification_request_status", make_id_filter("id", ids),
        std::back_inserter(rows)));
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    auto latest = std::unordered_map<
      AccountModificationRequest::Id, AccountModificationRequest::Update>();
    for(auto& row : rows) {
      auto i = latest.find(row.m_id);
      if(i == latest.end() ||
          i->second.m_sequence_number < row.m_update.m_sequence_number) {
        latest[row.m_id] = row.m_update;
      }
    }
    for(auto i = std::size_t(0); i != ids.size(); ++i) {
      auto entry = latest.find(ids[i]);
      if(entry != latest.end()) {
        statuses[i] = entry->second;
        statuses[i].m_account =
          m_directory_entries.load(statuses[i].m_account.m_id);
      }
    }
    return statuses;
  }

  template<typename C>
  std::vector<int> SqlAdministrationDataStore<C>::load_message_counts(
      const std::vector<AccountModificationRequest::Id>& ids) {
    auto counts = std::vector(ids.size(), 0);
    if(ids.empty()) {
      return counts;
    }
    auto rows = std::vector<AccountModificationRequestMessageIndex>();
    try {
      m_connection->execute(Viper::select(
        get_account_modification_request_message_index_row(),
        "account_modification_request_messages",
        make_id_filter("request_id", ids), std::back_inserter(rows)));
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    auto tally = std::unordered_map<AccountModificationRequest::Id, int>();
    for(auto& row : rows) {
      ++tally[row.m_request_id];
    }
    for(auto i = std::size_t(0); i != ids.size(); ++i) {
      auto entry = tally.find(ids[i]);
      if(entry != tally.end()) {
        counts[i] = entry->second;
      }
    }
    return counts;
  }

  template<typename C>
  std::vector<boost::optional<AccountModificationRequest::Id>>
      SqlAdministrationDataStore<C>::load_previous_granted_requests(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto predecessors =
      std::vector<boost::optional<AccountModificationRequest::Id>>(ids.size());
    if(ids.empty()) {
      return predecessors;
    }
    auto requests = std::vector<AccountModificationRequest>();
    try {
      m_connection->execute(Viper::select(
        get_account_modification_request_row(),
        "account_modification_requests", make_id_filter("id", ids),
        std::back_inserter(requests)));
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    auto index = std::unordered_map<AccountModificationRequest::Id,
      const AccountModificationRequest*>();
    index.reserve(requests.size());
    for(auto& request : requests) {
      index[request.get_id()] = &request;
    }
    auto accounts = std::vector<unsigned int>();
    auto types = std::vector<AccountModificationRequest::Type>();
    auto ceiling = AccountModificationRequest::Id(0);
    for(auto& request : requests) {
      accounts.push_back(request.get_account().m_id);
      types.push_back(request.get_type());
      ceiling = std::max(ceiling, request.get_id());
    }
    std::ranges::sort(accounts);
    accounts.erase(std::ranges::unique(accounts).begin(), accounts.end());
    std::ranges::sort(types);
    types.erase(std::ranges::unique(types).begin(), types.end());
    if(accounts.empty()) {
      return predecessors;
    }
    auto candidates = std::vector<RequestIndex>();
    try {
      m_connection->execute(Viper::select(get_request_index_row(),
        "account_modification_requests",
        Viper::in(Viper::sym("account"), accounts.begin(), accounts.end()) &&
          Viper::in(Viper::sym("type"), types.begin(), types.end()) &&
          Viper::sym("status") == AccountModificationRequest::Status::GRANTED &&
          Viper::sym("id") < ceiling,
        std::back_inserter(candidates)));
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    auto history = std::map<std::pair<unsigned int,
      AccountModificationRequest::Type>,
      std::vector<AccountModificationRequest::Id>>();
    for(auto& candidate : candidates) {
      history[std::pair(candidate.m_account.m_id, candidate.m_type)].
        push_back(candidate.m_id);
    }
    for(auto& entry : history) {
      std::ranges::sort(entry.second);
    }
    for(auto i = std::size_t(0); i != ids.size(); ++i) {
      auto entry = index.find(ids[i]);
      if(entry == index.end()) {
        continue;
      }
      auto& request = *entry->second;
      auto candidate =
        history.find(std::pair(request.get_account().m_id, request.get_type()));
      if(candidate == history.end()) {
        continue;
      }
      auto position = std::ranges::lower_bound(candidate->second, ids[i]);
      if(position != candidate->second.begin()) {
        predecessors[i] = *std::prev(position);
      }
    }
    return predecessors;
  }

  template<typename C>
  std::vector<EntitlementModification>
      SqlAdministrationDataStore<C>::load_entitlement_modifications(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto modifications = std::vector<EntitlementModification>(ids.size());
    if(ids.empty()) {
      return modifications;
    }
    auto rows = std::vector<EntitlementModificationRow>();
    try {
      m_connection->execute(Viper::select(
        get_entitlement_modification_row(), "entitlement_modifications",
        make_id_filter("id", ids), std::back_inserter(rows)));
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    auto entitlements = std::unordered_map<
      AccountModificationRequest::Id, std::vector<Beam::DirectoryEntry>>();
    for(auto& row : rows) {
      entitlements[row.m_id].push_back(
        m_directory_entries.load(row.m_entitlement.m_id));
    }
    for(auto i = std::size_t(0); i != ids.size(); ++i) {
      auto entry = entitlements.find(ids[i]);
      if(entry != entitlements.end()) {
        modifications[i] = EntitlementModification(entry->second);
      }
    }
    return modifications;
  }

  template<typename C>
  std::vector<RiskModification>
      SqlAdministrationDataStore<C>::load_risk_modifications(
        const std::vector<AccountModificationRequest::Id>& ids) {
    auto modifications = std::vector<RiskModification>(ids.size());
    if(ids.empty()) {
      return modifications;
    }
    auto rows = std::vector<IndexedRiskModification>();
    try {
      m_connection->execute(
        Viper::select(get_risk_modification_row(), "risk_modifications",
          make_id_filter("id", ids), std::back_inserter(rows)));
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    auto parameters =
      std::unordered_map<AccountModificationRequest::Id, RiskParameters>();
    parameters.reserve(rows.size());
    for(auto& row : rows) {
      parameters[row.m_id] = row.m_parameters;
    }
    for(auto i = std::size_t(0); i != ids.size(); ++i) {
      auto entry = parameters.find(ids[i]);
      if(entry != parameters.end()) {
        modifications[i] = RiskModification(entry->second);
      }
    }
    return modifications;
  }

  template<typename C>
  EntitlementModification
      SqlAdministrationDataStore<C>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    auto entitlements = std::vector<Beam::DirectoryEntry>();
    try {
      auto ids = std::vector<unsigned int>();
      m_connection->execute(Viper::select(
        Viper::Row<unsigned int>("entitlement"), "entitlement_modifications",
        Viper::sym("id") == id, std::back_inserter(ids)));
      for(auto& id : ids) {
        entitlements.push_back(m_directory_entries.load(id));
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return entitlements;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store_effective_date(
      AccountModificationRequest::Id id,
      boost::posix_time::ptime effective_date) {
    try {
      m_connection->execute(Viper::update(
        "account_modification_requests",
        Viper::SetClause("effective_date", effective_date),
        Viper::sym("id") == id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    auto entitlements = std::vector<EntitlementModificationRow>();
    for(auto& entitlement : modification.get_entitlements()) {
      entitlements.push_back({request.get_id(), entitlement});
    }
    auto stored =
      StoredAccountModificationRequest(request, request.get_timestamp(),
        AccountModificationRequest::Status::NONE);
    try {
      m_connection->execute(Viper::insert(
        get_stored_account_modification_request_row(),
        "account_modification_requests", &stored));
      m_connection->execute(Viper::insert(get_entitlement_modification_row(),
        "entitlement_modifications", entitlements.begin(), entitlements.end()));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  RiskModification SqlAdministrationDataStore<C>::load_risk_modification(
      AccountModificationRequest::Id id) {
    auto parameters = RiskParameters();
    try {
      m_connection->execute(Viper::select(get_risk_parameters_row(),
        "risk_modifications", Viper::sym("id") == id, &parameters));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return {std::move(parameters)};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    auto indexed_modification = IndexedRiskModification(
      request.get_id(), request.get_account(), modification.get_parameters());
    auto stored =
      StoredAccountModificationRequest(request, request.get_timestamp(),
        AccountModificationRequest::Status::NONE);
    try {
      m_connection->execute(
        Viper::insert(get_stored_account_modification_request_row(),
          "account_modification_requests", &stored));
      m_connection->execute(Viper::insert(get_risk_modification_row(),
        "risk_modifications", &indexed_modification));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(
      AccountModificationRequest::Id id, const Message& message) {
    auto index = AdministrationMessageIndex(
      message.get_id(), message.get_account(), message.get_timestamp());
    auto bodies = std::vector<IndexedMessageBody>();
    for(auto& body : message.get_bodies()) {
      bodies.push_back({message.get_id(), body});
    }
    auto modification_index =
      AccountModificationRequestMessageIndex(id, message.get_id());
    try {
      m_connection->execute(Viper::insert(
        get_administration_message_index_row(), "administration_messages",
        &index));
      m_connection->execute(Viper::insert(get_indexed_message_body_row(),
        "administration_message_bodies", bodies.begin(), bodies.end()));
      m_connection->execute(Viper::insert(
        get_account_modification_request_message_index_row(),
        "account_modification_request_messages", &modification_index));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  AccountModificationRequest::Update
      SqlAdministrationDataStore<C>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    auto status = AccountModificationRequest::Update();
    try {
      m_connection->execute(Viper::select(
        get_account_modification_request_status_row(),
        "account_modification_request_status", Viper::sym("id") == id,
        Viper::order_by("sequence_number", Viper::Order::DESC), Viper::limit(1),
        &status));
      status.m_account = m_directory_entries.load(status.m_account.m_id);
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return status;
  }

  template<typename C>
  std::vector<AccountModificationRequest::Update>
      SqlAdministrationDataStore<C>::load_account_modification_request_updates(
        AccountModificationRequest::Id id) {
    auto updates = std::vector<AccountModificationRequest::Update>();
    try {
      m_connection->execute(Viper::select(
        get_account_modification_request_status_row(),
        "account_modification_request_status", Viper::sym("id") == id,
        Viper::order_by("sequence_number", Viper::Order::ASC),
        std::back_inserter(updates)));
      for(auto& update : updates) {
        update.m_account = m_directory_entries.load(update.m_account.m_id);
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return updates;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    auto indexed_updated = IndexedAccountModificationRequestStatus(id, status);
    try {
      m_connection->execute(
        Viper::insert(get_indexed_account_modification_request_status(),
          "account_modification_request_status", &indexed_updated));
      m_connection->execute(Viper::update("account_modification_requests",
        Viper::SetClause("last_update_timestamp", status.m_timestamp),
        Viper::sym("id") == id));
      m_connection->execute(Viper::update("account_modification_requests",
        Viper::SetClause("status", status.m_status), Viper::sym("id") == id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  Message::Id SqlAdministrationDataStore<C>::load_last_message_id() {
    auto id = std::optional<Message::Id>();
    try {
      m_connection->execute(Viper::select(Viper::Row<Message::Id>("id"),
        "administration_messages", Viper::order_by("id", Viper::Order::DESC),
        Viper::limit(1), &id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    if(!id) {
      return 0;
    }
    return *id;
  }

  template<typename C>
  Message SqlAdministrationDataStore<C>::load_message(Message::Id id) {
    auto index = std::optional<AdministrationMessageIndex>();
    try {
      m_connection->execute(Viper::select(
        get_administration_message_index_row(), "administration_messages",
        Viper::sym("id") == id, &index));
      if(index) {
        index->m_account = m_directory_entries.load(index->m_account.m_id);
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    if(!index) {
      return {};
    }
    auto bodies = std::vector<Message::Body>();
    try {
      m_connection->execute(Viper::select(get_message_body_row(),
        "administration_message_bodies", Viper::sym("id") == id,
        std::back_inserter(bodies)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return Message(
      index->m_id, index->m_account, index->m_timestamp, std::move(bodies));
  }

  template<typename C>
  std::vector<Message::Id> SqlAdministrationDataStore<C>::load_message_ids(
      AccountModificationRequest::Id id) {
    auto ids = std::vector<Message::Id>();
    try {
      m_connection->execute(Viper::select(Viper::Row<Message::Id>("message_id"),
        "account_modification_request_messages", Viper::sym("request_id") == id,
        Viper::order_by("message_id", Viper::Order::ASC),
        std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return ids;
  }

  template<typename C>
  std::vector<Notification> SqlAdministrationDataStore<C>::load_notifications(
      const Beam::DirectoryEntry& account, const Notification::Id& id,
      Beam::SnapshotLimit limit, Notification::ReadState read_state) {
    auto notifications = std::vector<Notification>();
    try {
      auto condition = Viper::sym("account") == account.m_id;
      if(read_state == Notification::ReadState::UNREAD) {
        condition = condition && Viper::sym("is_read") == false;
      } else if(read_state == Notification::ReadState::READ) {
        condition = condition && Viper::sym("is_read") == true;
      }
      if(!id.empty()) {
        if(limit.get_type() == Beam::SnapshotLimit::Type::TAIL) {
          condition = condition && Viper::sym("id") <= id;
        } else {
          condition = condition && Viper::sym("id") >= id;
        }
      }
      auto order = [&] {
        if(limit.get_type() == Beam::SnapshotLimit::Type::TAIL) {
          return Viper::Order::DESC;
        }
        return Viper::Order::ASC;
      }();
      m_connection->execute(Viper::select(get_notification_row(),
        "notifications", std::move(condition),
        Viper::order_by("timestamp", order), Viper::limit(limit.get_size()),
        std::back_inserter(notifications)));
      for(auto& notification : notifications) {
        notification.m_account =
          m_directory_entries.load(notification.m_account.m_id);
      }
      if(limit.get_type() == Beam::SnapshotLimit::Type::TAIL) {
        std::ranges::reverse(notifications);
      }
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return notifications;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::mark_notification_as_read(
      const Notification::Id& id) {
    try {
      m_connection->execute(Viper::update("notifications",
        Viper::SetClause("is_read", true), Viper::sym("id") == id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::mark_notification_as_unread(
      const Notification::Id& id) {
    try {
      m_connection->execute(Viper::update("notifications",
        Viper::SetClause("is_read", false), Viper::sym("id") == id));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::store(const Notification& notification) {
    try {
      m_connection->execute(Viper::insert(
        get_notification_row(), "notifications", &notification));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  template<typename F>
  decltype(auto) SqlAdministrationDataStore<C>::with_transaction(
      F&& transaction) {
    auto lock = std::lock_guard(m_mutex);
    return Viper::transaction(*m_connection, std::forward<F>(transaction));
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_connection->close();
    m_open_state.close();
  }
}

#endif
