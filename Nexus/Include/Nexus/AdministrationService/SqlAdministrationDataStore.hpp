#ifndef NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/AdministrationService/SqlDefinitions.hpp"

namespace Nexus::AdministrationService {

  /**
   * Stores account administration data in a MySQL database.
   * @param <C> The type of SQL connection.
   */
  template<typename C>
  class SqlAdministrationDataStore : public AdministrationDataStore {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /** The function used to load DirectoryEntries. */
      using DirectoryEntrySourceFunction = Beam::KeyValueCache<unsigned int,
        Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex>::SourceFunction;

      /**
       * Constructs an SqlAdministrationDataStore.
       * @param connection The connection to the SQL database.
       * @param directoryEntrySourceFunction The function used to load
       *        DirectoryEntries.
       */
      SqlAdministrationDataStore(std::unique_ptr<Connection> connection,
        const DirectoryEntrySourceFunction& directoryEntrySourceFunction);

      ~SqlAdministrationDataStore() override;

      std::vector<IndexedAccountIdentity> LoadAllAccountIdentities() override;

      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      std::vector<IndexedRiskParameters> LoadAllRiskParameters() override;

      RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      std::vector<IndexedRiskState> LoadAllRiskStates() override;

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

      void Open() override;

      void Close() override;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_directoryEntries;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlAdministrationDataStore<C>::SqlAdministrationDataStore(
    std::unique_ptr<Connection> connection,
    const DirectoryEntrySourceFunction& directoryEntrySourceFunction)
    : m_connection(std::move(connection)),
      m_directoryEntries(directoryEntrySourceFunction) {}

  template<typename C>
  SqlAdministrationDataStore<C>::~SqlAdministrationDataStore() {
    Close();
  }

  template<typename C>
  std::vector<AdministrationDataStore::IndexedAccountIdentity>
      SqlAdministrationDataStore<C>::LoadAllAccountIdentities() {
    auto identities = std::vector<IndexedAccountIdentity>();
    try {
      m_connection->execute(Viper::select(GetIndexedAccountIdentityRow(),
        "account_identities", std::back_inserter(identities)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return identities;
  }

  template<typename C>
  AccountIdentity SqlAdministrationDataStore<C>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto identity = AccountIdentity();
    try {
      m_connection->execute(Viper::select(GetAccountIdentityRow(),
        "account_identities", Viper::sym("account") == account.m_id,
        &identity));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return identity;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto row = IndexedAccountIdentity{account, identity};
    try {
      m_connection->execute(Viper::upsert(GetIndexedAccountIdentityRow(),
        "account_identities", &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<AdministrationDataStore::IndexedRiskParameters>
      SqlAdministrationDataStore<C>::LoadAllRiskParameters() {
    auto parameters = std::vector<IndexedRiskParameters>();
    try {
      m_connection->execute(Viper::select(GetIndexedRiskParametersRow(),
        "risk_parameters", std::back_inserter(parameters)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return parameters;
  }

  template<typename C>
  RiskService::RiskParameters SqlAdministrationDataStore<C>::LoadRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parameters = RiskService::RiskParameters();
    try {
      m_connection->execute(Viper::select(GetRiskParametersRow(),
        "risk_parameters", Viper::sym("account") == account.m_id, &parameters));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return parameters;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto parameters = IndexedRiskParameters{account, riskParameters};
    try {
      m_connection->execute(Viper::upsert(GetIndexedRiskParametersRow(),
        "risk_parameters", &parameters));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<AdministrationDataStore::IndexedRiskState>
      SqlAdministrationDataStore<C>::LoadAllRiskStates() {
    auto states = std::vector<IndexedRiskState>();
    try {
      m_connection->execute(Viper::select(GetIndexedRiskStateRow(),
        "risk_states", std::back_inserter(states)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return states;
  }

  template<typename C>
  RiskService::RiskState SqlAdministrationDataStore<C>::LoadRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto state = RiskService::RiskState();
    try {
      m_connection->execute(Viper::select(GetRiskStateRow(),
        "risk_states", Viper::sym("account") == account.m_id, &state));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return state;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto indexedState = IndexedRiskState{account, riskState};
    try {
      m_connection->execute(Viper::upsert(GetIndexedRiskStateRow(),
        "risk_states", &indexedState));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  AccountModificationRequest SqlAdministrationDataStore<C>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    auto request = AccountModificationRequest();
    try {
      m_connection->execute(Viper::select(GetAccountModificationRequestRow(),
        "account_modification_requests", Viper::sym("id") == id, &request));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return request;
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      SqlAdministrationDataStore<C>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    if(startId == -1) {
      startId = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    maxCount = std::min(maxCount, 1000);
    auto ids = std::vector<AccountModificationRequest::Id>();
    try {
      m_connection->execute(Viper::select(
        Viper::Row<AccountModificationRequest::Id>("id"),
        "account_modification_requests", Viper::sym("id") < startId &&
        Viper::sym("account") == account.m_id,
        Viper::order_by("id", Viper::Order::DESC), Viper::limit(maxCount),
        std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return ids;
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      SqlAdministrationDataStore<C>::LoadAccountModificationRequestIds(
      AccountModificationRequest::Id startId, int maxCount) {
    if(startId == -1) {
      startId = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    maxCount = std::min(maxCount, 1000);
    auto ids = std::vector<AccountModificationRequest::Id>();
    try {
      m_connection->execute(Viper::select(
        Viper::Row<AccountModificationRequest::Id>("id"),
        "account_modification_requests", Viper::sym("id") < startId,
        Viper::order_by("id", Viper::Order::DESC), Viper::limit(maxCount),
        std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return ids;
  }

  template<typename C>
  EntitlementModification
      SqlAdministrationDataStore<C>::LoadEntitlementModification(
      AccountModificationRequest::Id id) {
    auto ids = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select(
        Viper::Row<unsigned int>("entitlement"), "entitlement_modifications",
        Viper::sym("id") == id, std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    auto entitlements = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& id : ids) {
      entitlements.push_back(m_directoryEntries.Load(id));
    }
    return entitlements;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    auto entitlements = std::vector<EntitlementModificationRow>();
    for(auto& entitlement : modification.GetEntitlements()) {
      entitlements.push_back({request.GetId(), entitlement});
    }
    try {
      m_connection->execute(Viper::insert(GetAccountModificationRequestRow(),
        "account_modification_requests", &request));
      m_connection->execute(Viper::insert(GetEntitlementModificationRow(),
        "entitlement_modifications", entitlements.begin(), entitlements.end()));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  RiskModification SqlAdministrationDataStore<C>::LoadRiskModification(
      AccountModificationRequest::Id id) {
    auto parameters = RiskService::RiskParameters();
    try {
      m_connection->execute(Viper::select(GetRiskParametersRow(),
        "risk_modifications", Viper::sym("id") == id, &parameters));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return {std::move(parameters)};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {
    auto indexedModification = IndexedRiskModification{request.GetId(),
      request.GetAccount(), modification.GetParameters()};
    try {
      m_connection->execute(Viper::insert(GetAccountModificationRequestRow(),
        "account_modification_requests", &request));
      m_connection->execute(Viper::insert(GetRiskModificationRow(),
        "risk_modifications", &indexedModification));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(AccountModificationRequest::Id id,
      const Message& message) {
    auto index = AdministrationMessageIndex{id, message.GetAccount(),
      message.GetTimestamp()};
    auto bodies = std::vector<IndexedMessageBody>();
    for(auto& body : message.GetBodies()) {
      bodies.push_back({message.GetId(), body});
    }
    auto modificationIndex = AccountModificationRequestMessageIndex{id,
      message.GetId()};
    try {
      m_connection->execute(Viper::insert(GetAdministrationMessageIndexRow(),
        "administration_messages", &index));
      m_connection->execute(Viper::insert(GetIndexedMessageBodyRow(),
        "administration_message_bodies", bodies.begin(), bodies.end()));
      m_connection->execute(Viper::insert(
        GetAccountModificationRequestMessageIndexRow(),
        "account_modification_request_messages", &modificationIndex));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  AccountModificationRequest::Update
      SqlAdministrationDataStore<C>::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    auto status = AccountModificationRequest::Update();
    try {
      m_connection->execute(Viper::select(
        GetAccountModificationRequestStatusRow(),
        "account_modification_request_status", Viper::sym("id") == id,
        Viper::order_by("sequence_number", Viper::Order::DESC),
        Viper::limit(1), &status));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return status;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    auto indexedUpdated = IndexedAccountModificationRequestStatus{id, status};
    try {
      m_connection->execute(Viper::insert(
        GetIndexedAccountModificationRequestStatus(),
        "account_modification_request_status", &indexedUpdated));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  Message::Id SqlAdministrationDataStore<C>::LoadLastMessageId() {
    auto id = std::optional<Message::Id>();
    try {
      m_connection->execute(Viper::select(Viper::Row<Message::Id>("id"),
        "administration_messages", Viper::order_by("id", Viper::Order::DESC),
        Viper::limit(1), &id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    if(!id) {
      return 0;
    }
    return *id;
  }

  template<typename C>
  Message SqlAdministrationDataStore<C>::LoadMessage(Message::Id id) {
    auto index = std::optional<AdministrationMessageIndex>();
    try {
      m_connection->execute(Viper::select(GetAdministrationMessageIndexRow(),
        "administration_messages", Viper::sym("id") == id, &index));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    if(!index) {
      return {};
    }
    auto bodies = std::vector<Message::Body>();
    try {
      m_connection->execute(Viper::select(GetMessageBodyRow(),
        "administration_message_bodies", Viper::sym("id") == id,
        std::back_inserter(bodies)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return Message(index->m_id, index->m_account, index->m_timestamp,
      std::move(bodies));
  }

  template<typename C>
  std::vector<Message::Id> SqlAdministrationDataStore<C>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    auto ids = std::vector<Message::Id>();
    try {
      m_connection->execute(Viper::select(Viper::Row<Message::Id>("message_id"),
        "account_modification_request_messages", Viper::sym("request_id") == id,
        Viper::order_by("message_id", Viper::Order::ASC),
        std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    return ids;
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, transaction);
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        GetIndexedAccountIdentityRow(), "account_identities"));
      m_connection->execute(Viper::create_if_not_exists(
        GetIndexedRiskParametersRow(), "risk_parameters"));
      m_connection->execute(Viper::create_if_not_exists(
        GetIndexedRiskStateRow(), "risk_states"));
      m_connection->execute(Viper::create_if_not_exists(
        GetAccountModificationRequestRow(), "account_modification_requests"));
      m_connection->execute(Viper::create_if_not_exists(
        GetEntitlementModificationRow(), "entitlement_modifications"));
      m_connection->execute(Viper::create_if_not_exists(
        GetRiskModificationRow(), "risk_modifications"));
      m_connection->execute(Viper::create_if_not_exists(
        GetIndexedAccountModificationRequestStatus(),
        "account_modification_request_status"));
      m_connection->execute(Viper::create_if_not_exists(
        GetAdministrationMessageIndexRow(), "administration_messages"));
      m_connection->execute(Viper::create_if_not_exists(
        GetIndexedMessageBodyRow(), "administration_message_bodies"));
      m_connection->execute(Viper::create_if_not_exists(
        GetAccountModificationRequestMessageIndexRow(),
        "account_modification_request_messages"));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Shutdown() {
    m_connection->close();
    m_openState.SetClosed();
  }
}

#endif
