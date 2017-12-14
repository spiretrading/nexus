#ifndef NEXUS_MYSQLADMINISTRATIONDATASTORE_HPP
#define NEXUS_MYSQLADMINISTRATIONDATASTORE_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/MySql/PosixTimeToMySqlDateTime.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/AdministrationService/MySqlAdministrationDataStoreDetails.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class MySqlAdministrationDataStore
      \brief Stores account administration data in a MySQL database.
   */
  class MySqlAdministrationDataStore : public AdministrationDataStore {
    public:

      //! The function used to load DirectoryEntries.
      using DirectoryEntrySourceFunction = Beam::KeyValueCache<unsigned int,
        Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex>::SourceFunction;

      //! Constructs a MySqlAdministrationDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
        \param directoryEntrySourceFunction The function used to load
               DirectoryEntries.
      */
      MySqlAdministrationDataStore(const Beam::Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password,
        const DirectoryEntrySourceFunction& directoryEntrySourceFunction);

      virtual ~MySqlAdministrationDataStore() override;

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

      virtual void Store(const AccountModificationRequest& request,
        const EntitlementModification& modification) override;

      virtual void Store(AccountModificationRequest::Id id,
        const Message& message) override;

      virtual AccountModificationRequest::Update
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      virtual void Store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status) override;

      virtual Message::Id LoadLastMessageId() override;

      virtual Message LoadMessage(Message::Id id) override;

      virtual std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_directoryEntries;
      mysqlpp::Connection m_databaseConnection;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline MySqlAdministrationDataStore::MySqlAdministrationDataStore(
      const Beam::Network::IpAddress& address, const std::string& schema,
      const std::string& username, const std::string& password,
      const DirectoryEntrySourceFunction& directoryEntrySourceFunction)
      : m_address{address},
        m_schema{schema},
        m_username{username},
        m_password{password},
        m_directoryEntries{directoryEntrySourceFunction},
        m_databaseConnection{false} {}

  inline MySqlAdministrationDataStore::~MySqlAdministrationDataStore() {
    Close();
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      AccountIdentity>> MySqlAdministrationDataStore::
      LoadAllAccountIdentities() {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM account_identities";
    std::vector<Details::account_identities> identities;
    query.storein(identities);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      AccountIdentity>> results;
    results.reserve(identities.size());
    std::transform(identities.begin(), identities.end(),
      std::back_inserter(results),
      [] (auto& row) {
        auto account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
          row.account);
        AccountIdentity identity;
        identity.m_firstName = row.first_name;
        identity.m_lastName = row.last_name;
        identity.m_emailAddress = row.e_mail;
        identity.m_addressLineOne = row.address_line_one;
        identity.m_addressLineTwo = row.address_line_two;
        identity.m_addressLineThree = row.address_line_three;
        identity.m_city = row.city;
        identity.m_province = row.province;
        identity.m_country = static_cast<CountryCode>(row.country);
        identity.m_userNotes = row.user_notes;
        return std::make_tuple(account, identity);
      });
    return results;
  }

  inline AccountIdentity MySqlAdministrationDataStore::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM account_identities WHERE account = " <<
      account.m_id;
    std::vector<Details::account_identities> identities;
    query.storein(identities);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    AccountIdentity result;
    if(identities.empty()) {
      return result;
    }
    auto& row = identities.front();
    result.m_firstName = row.first_name;
    result.m_lastName = row.last_name;
    result.m_emailAddress = row.e_mail;
    result.m_addressLineOne = row.address_line_one;
    result.m_addressLineTwo = row.address_line_two;
    result.m_addressLineThree = row.address_line_three;
    result.m_city = row.city;
    result.m_province = row.province;
    result.m_country = static_cast<CountryCode>(row.country);
    result.m_userNotes = row.user_notes;
    return result;
  }

  inline void MySqlAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto query = m_databaseConnection.query();
    Details::account_identities entryRow{account.m_id, identity.m_firstName,
      identity.m_lastName, identity.m_emailAddress, identity.m_addressLineOne,
      identity.m_addressLineTwo, identity.m_addressLineThree, identity.m_city,
      identity.m_province, identity.m_country, identity.m_userNotes, ""};
    query.replace(entryRow);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>> MySqlAdministrationDataStore::
      LoadAllRiskParameters() {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM risk_parameters";
    std::vector<Details::risk_parameters> riskParameters;
    query.storein(riskParameters);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>> results;
    results.reserve(riskParameters.size());
    std::transform(riskParameters.begin(), riskParameters.end(),
      std::back_inserter(results),
      [] (auto& row) {
        auto account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
          row.account);
        RiskService::RiskParameters parameters;
        parameters.m_currency = CurrencyId{row.currency};
        parameters.m_buyingPower = Money::FromRepresentation(row.buying_power);
        parameters.m_allowedState = static_cast<RiskService::RiskState::Type>(
          row.allowed_state);
        parameters.m_netLoss = Money::FromRepresentation(row.net_loss);
        parameters.m_lossFromTop = row.loss_from_top;
        parameters.m_transitionTime = boost::posix_time::seconds(
          row.transition_time);
        return std::make_tuple(account, parameters);
      });
    return results;
  }

  inline RiskService::RiskParameters MySqlAdministrationDataStore::
      LoadRiskParameters(const Beam::ServiceLocator::DirectoryEntry& account) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM risk_parameters WHERE account = " << account.m_id;
    std::vector<Details::risk_parameters> riskParameters;
    query.storein(riskParameters);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    RiskService::RiskParameters result;
    if(riskParameters.empty()) {
      return result;
    }
    auto& row = riskParameters.front();
    result.m_currency = CurrencyId{row.currency};
    result.m_buyingPower = Money::FromRepresentation(row.buying_power);
    result.m_allowedState = static_cast<RiskService::RiskState::Type>(
      row.allowed_state);
    result.m_netLoss = Money::FromRepresentation(row.net_loss);
    result.m_lossFromTop = row.loss_from_top;
    result.m_transitionTime = boost::posix_time::seconds(row.transition_time);
    return result;
  }

  inline void MySqlAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto query = m_databaseConnection.query();
    Details::risk_parameters entryRow{account.m_id,
      riskParameters.m_currency.m_value,
      riskParameters.m_buyingPower.GetRepresentation(),
      riskParameters.m_netLoss.GetRepresentation(),
      static_cast<int>(riskParameters.m_allowedState.m_type),
      riskParameters.m_lossFromTop,
      static_cast<int>(riskParameters.m_transitionTime.total_seconds())};
    query.replace(entryRow);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
  }

  inline std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> MySqlAdministrationDataStore::
      LoadAllRiskStates() {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM risk_states";
    std::vector<Details::risk_states> riskStates;
    query.storein(riskStates);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> results;
    results.reserve(riskStates.size());
    std::transform(riskStates.begin(), riskStates.end(),
      std::back_inserter(results),
      [] (auto& row) {
        auto account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
          row.account);
        RiskService::RiskState state{static_cast<RiskService::RiskState::Type>(
          row.state), Beam::MySql::FromDateTime(row.expiry)};
        return std::make_tuple(account, state);
      });
    return results;
  }

  inline RiskService::RiskState MySqlAdministrationDataStore::LoadRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM risk_states WHERE account = " << account.m_id;
    std::vector<Details::risk_states> riskStates;
    query.storein(riskStates);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    if(riskStates.empty()) {
      return RiskService::RiskState();
    }
    auto row = riskStates.front();
    RiskService::RiskState result{static_cast<RiskService::RiskState::Type>(
      row.state), Beam::MySql::FromDateTime(row.expiry)};
    return result;
  }

  inline void MySqlAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto query = m_databaseConnection.query();
    Details::risk_states entryRow{account.m_id,
      static_cast<int>(riskState.m_type),
      Beam::MySql::ToDateTime(riskState.m_expiry)};
    query.replace(entryRow);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
  }

  inline AccountModificationRequest MySqlAdministrationDataStore::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM account_modification_requests WHERE id = " << id;
    std::vector<Details::account_modification_requests> rows;
    query.storein(rows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    if(rows.empty()) {
      return {};
    }
    auto& row = rows.front();
    AccountModificationRequest result{row.id,
      static_cast<AccountModificationRequest::Type>(row.type),
      m_directoryEntries.Load(row.account),
      m_directoryEntries.Load(row.submission_account),
      Beam::MySql::FromMySqlTimestamp(row.timestamp)};
    return result;
  }

  inline std::vector<AccountModificationRequest::Id>
      MySqlAdministrationDataStore::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    if(startId == -1) {
      startId = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    maxCount = std::min(maxCount, 1000);
    auto query = m_databaseConnection.query();
    query << "SELECT id FROM account_modification_requests WHERE id < " <<
      startId << " AND account = " << account.m_id <<
      " ORDER BY id DESC LIMIT " << maxCount;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<AccountModificationRequest::Id> rows;
    for(auto& row : result) {
      rows.push_back(row[0]);
    }
    return rows;
  }

  inline std::vector<AccountModificationRequest::Id>
      MySqlAdministrationDataStore::LoadAccountModificationRequestIds(
      AccountModificationRequest::Id startId, int maxCount) {
    if(startId == -1) {
      startId = std::numeric_limits<AccountModificationRequest::Id>::max();
    }
    maxCount = std::min(maxCount, 1000);
    auto query = m_databaseConnection.query();
    query << "SELECT id FROM account_modification_requests WHERE id < " <<
      startId << " ORDER BY id DESC LIMIT " << maxCount;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<AccountModificationRequest::Id> rows;
    for(auto& row : result) {
      rows.push_back(row[0]);
    }
    return rows;
  }

  inline EntitlementModification MySqlAdministrationDataStore::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    auto query = m_databaseConnection.query();
    query << "SELECT entitlement FROM entitlement_modifications WHERE id = " <<
      id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<Beam::ServiceLocator::DirectoryEntry> rows;
    for(auto& row : result) {
      rows.push_back(m_directoryEntries.Load(row[0]));
    }
    return EntitlementModification{std::move(rows)};
  }

  inline void MySqlAdministrationDataStore::Store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {
    auto query = m_databaseConnection.query();
    Details::account_modification_requests row{request.GetId(),
      static_cast<int>(request.GetType()), request.GetAccount().m_id,
      request.GetSubmissionAccount().m_id,
      Beam::MySql::ToMySqlTimestamp(request.GetTimestamp())};
    query.insert(row);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    if(!modification.GetEntitlements().empty()) {
      query.reset();
      std::vector<Details::entitlement_modifications> entitlements;
      for(auto& entitlement : modification.GetEntitlements()) {
        entitlements.emplace_back(request.GetId(), entitlement.m_id);
      }
      query.insert(entitlements.begin(), entitlements.end());
      if(!query.execute()) {
        BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
      }
    }
  }

  inline void MySqlAdministrationDataStore::Store(
      AccountModificationRequest::Id id, const Message& message) {
    auto query = m_databaseConnection.query();
    Details::administration_messages row{message.GetId(),
      message.GetAccount().m_id,
      Beam::MySql::ToMySqlTimestamp(message.GetTimestamp())};
    query.insert(row);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    query.reset();
    std::vector<Details::administration_message_bodies> bodies;
    for(auto& body : message.GetBodies()) {
      bodies.emplace_back(message.GetId(), body.m_contentType, body.m_message);
    }
    query.insert(bodies.begin(), bodies.end());
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    query.reset();
    Details::account_modification_request_messages requestId{
      id, message.GetId()};
    query.insert(requestId);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
  }

  inline AccountModificationRequest::Update
      MySqlAdministrationDataStore::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM account_modification_request_status WHERE id = " <<
      id << " ORDER BY sequence_number DESC LIMIT 1";
    std::vector<Details::account_modification_request_status> rows;
    query.storein(rows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    if(rows.empty()) {
      return {};
    }
    auto& row = rows.front();
    AccountModificationRequest::Update result{
      static_cast<AccountModificationRequest::Status>(row.status),
      m_directoryEntries.Load(row.account), row.sequence_number,
      Beam::MySql::FromMySqlTimestamp(row.timestamp)};
    return result;
  }

  inline void MySqlAdministrationDataStore::Store(
      AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {
    auto query = m_databaseConnection.query();
    Details::account_modification_request_status row{id,
      static_cast<int>(status.m_status), status.m_account.m_id,
      status.m_sequenceNumber,
      Beam::MySql::ToMySqlTimestamp(status.m_timestamp)};
    query.insert(row);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
  }

  inline Message::Id MySqlAdministrationDataStore::LoadLastMessageId() {
    auto query = m_databaseConnection.query();
    query << "SELECT id FROM administration_messages ORDER BY id DESC LIMIT 1";
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    if(result.empty()) {
      return 0;
    }
    return result[0][0];
  }

  inline Message MySqlAdministrationDataStore::LoadMessage(Message::Id id) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM administration_messages WHERE id = " << id;
    std::vector<Details::administration_messages> messageRows;
    query.storein(messageRows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    if(messageRows.empty()) {
      return {};
    }
    query.reset();
    query << "SELECT * FROM administration_message_bodies WHERE id = " << id;
    std::vector<Details::administration_message_bodies> bodyRows;
    query.storein(bodyRows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<Message::Body> bodies;
    for(auto& bodyRow : bodyRows) {
      bodies.push_back(Message::Body{std::move(bodyRow.content_type),
        std::move(bodyRow.message)});
    }
    auto& messageRow = messageRows.front();
    Message message{id, m_directoryEntries.Load(messageRow.account),
      Beam::MySql::FromMySqlTimestamp(messageRow.timestamp), std::move(bodies)};
    return message;
  }

  inline std::vector<Message::Id> MySqlAdministrationDataStore::LoadMessageIds(
      AccountModificationRequest::Id id) {
    auto query = m_databaseConnection.query();
    query << "SELECT message_id FROM account_modification_request_messages "
      "WHERE request_id = " << id << " ORDER BY message_id ASC";
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
    std::vector<Message::Id> rows;
    for(auto& row : result) {
      rows.push_back(row[0]);
    }
    return rows;
  }

  inline void MySqlAdministrationDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    mysqlpp::Transaction t{m_databaseConnection};
    try {
      transaction();
    } catch(...) {
      t.rollback();
      throw;
    }
    t.commit();
  }

  inline void MySqlAdministrationDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      auto connectionResult = m_databaseConnection.set_option(
        new mysqlpp::ReconnectOption{true});
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(Beam::IO::IOException{
          "Unable to set MySQL reconnect option."});
      }
      connectionResult = m_databaseConnection.connect(m_schema.c_str(),
        m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
        m_address.GetPort());
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(Beam::IO::ConnectException{std::string(
          "Unable to connect to MySQL database - ") +
          m_databaseConnection.error()});
      }
      if(!Details::LoadTables(m_databaseConnection, m_schema)) {
        BOOST_THROW_EXCEPTION(Beam::IO::IOException{
          "Unable to load database tables."});
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlAdministrationDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlAdministrationDataStore::Shutdown() {
    m_databaseConnection.disconnect();
    m_openState.SetClosed();
  }
}
}

#endif
