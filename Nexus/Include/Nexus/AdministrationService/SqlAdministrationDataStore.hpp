#ifndef NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/AdministrationService/SqlDefinitions.hpp"

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
      mutable Beam::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::KeyValueCache<unsigned int, Beam::DirectoryEntry, Beam::Mutex>
        m_directory_entries;
      Beam::OpenState m_open_state;
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
        get_account_modification_request_row(),
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
          request.get_timestamp());
    } catch(const std::exception& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return request;
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      SqlAdministrationDataStore<C>::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    if(start_id == -1) {
      start_id = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    max_count = std::min(max_count, 1000);
    auto ids = std::vector<AccountModificationRequest::Id>();
    try {
      m_connection->execute(Viper::select(
        Viper::Row<AccountModificationRequest::Id>("id"),
        "account_modification_requests", Viper::sym("id") > start_id &&
          Viper::sym("account") == account.m_id,
        Viper::order_by("id", Viper::Order::ASC), Viper::limit(max_count),
        std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return ids;
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      SqlAdministrationDataStore<C>::load_account_modification_request_ids(
        AccountModificationRequest::Id start_id, int max_count) {
    if(start_id == -1) {
      start_id = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    max_count = std::min(max_count, 1000);
    auto ids = std::vector<AccountModificationRequest::Id>();
    try {
      m_connection->execute(Viper::select(
        Viper::Row<AccountModificationRequest::Id>("id"),
        "account_modification_requests", Viper::sym("id") > start_id,
        Viper::order_by("id", Viper::Order::ASC), Viper::limit(max_count),
        std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      boost::throw_with_location(AdministrationDataStoreException(e.what()));
    }
    return ids;
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
  void SqlAdministrationDataStore<C>::store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    auto entitlements = std::vector<EntitlementModificationRow>();
    for(auto& entitlement : modification.get_entitlements()) {
      entitlements.push_back({request.get_id(), entitlement});
    }
    try {
      m_connection->execute(Viper::insert(
        get_account_modification_request_row(), "account_modification_requests",
        &request));
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
    try {
      m_connection->execute(Viper::insert(
        get_account_modification_request_row(), "account_modification_requests",
        &request));
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
  void SqlAdministrationDataStore<C>::store(AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    auto indexed_updated = IndexedAccountModificationRequestStatus(id, status);
    try {
      m_connection->execute(Viper::insert(
        get_indexed_account_modification_request_status(),
        "account_modification_request_status", &indexed_updated));
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
