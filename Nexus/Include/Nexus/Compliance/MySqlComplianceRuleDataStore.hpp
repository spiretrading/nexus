#ifndef NEXUS_MYSQLCOMPLIANCERULEDATASTORE_HPP
#define NEXUS_MYSQLCOMPLIANCERULEDATASTORE_HPP
#include <string>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/MySql/PosixTimeToMySqlDateTime.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStoreException.hpp"
#include "Nexus/Compliance/MySqlComplianceRuleDataStoreDetails.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class MySqlComplianceRuleDataStore
      \brief Implements a ComplianceRuleDataStore backed using MySQL.
   */
  class MySqlComplianceRuleDataStore : private boost::noncopyable {
    public:

      //! Constructs a MySqlComplianceRuleDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlComplianceRuleDataStore(Beam::Network::IpAddress address,
        std::string schema, std::string username, std::string password);

      ~MySqlComplianceRuleDataStore();

      ComplianceRuleId LoadNextComplianceRuleEntryId();

      boost::optional<ComplianceRuleEntry> LoadComplianceRuleEntry(
        ComplianceRuleId id);

      std::vector<ComplianceRuleEntry> LoadComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      void Store(const ComplianceRuleEntry& entry);

      void Delete(ComplianceRuleId id);

      void Store(const ComplianceRuleViolationRecord& violationRecord);

      void Open();

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      Beam::Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      mysqlpp::Connection m_databaseConnection;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      static ComplianceRuleEntry ConvertRow(
        const Details::compliance_rule_entries& row);
  };

  inline MySqlComplianceRuleDataStore::MySqlComplianceRuleDataStore(
      Beam::Network::IpAddress address, std::string schema,
      std::string username, std::string password)
      : m_address{std::move(address)},
        m_schema(std::move(schema)),
        m_username(std::move(username)),
        m_password(std::move(password)),
        m_databaseConnection{false} {}

  inline MySqlComplianceRuleDataStore::~MySqlComplianceRuleDataStore() {
    Close();
  }

  inline ComplianceRuleId MySqlComplianceRuleDataStore::
      LoadNextComplianceRuleEntryId() {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    query << "SELECT MAX(entry_id) FROM compliance_rule_entries";
    auto result = query.store();
    if(!result || result.size() != 1 || result[0][0].is_null()) {
      return 1;
    } else {
      return result[0][0].conv<std::uint64_t>(0) + 1;
    }
  }

  inline boost::optional<ComplianceRuleEntry> MySqlComplianceRuleDataStore::
      LoadComplianceRuleEntry(ComplianceRuleId id) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM compliance_rule_entries WHERE entry_id = " << id;
    std::vector<Details::compliance_rule_entries> rows;
    query.storein(rows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{query.error()});
    }
    if(rows.empty()) {
      return boost::none;
    }
    auto& row = rows.front();
    auto result = ConvertRow(row);
    return result;
  }

  inline std::vector<ComplianceRuleEntry>
      MySqlComplianceRuleDataStore::LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM compliance_rule_entries WHERE "
      "directory_entry = " << directoryEntry.m_id;
    std::vector<Details::compliance_rule_entries> rows;
    query.storein(rows);
    if(query.errnum() != 0) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{query.error()});
    }
    std::vector<ComplianceRuleEntry> complianceRuleEntries;
    std::transform(rows.begin(), rows.end(),
      std::back_inserter(complianceRuleEntries), ConvertRow);
    return complianceRuleEntries;
  }

  inline void MySqlComplianceRuleDataStore::Store(
      const ComplianceRuleEntry& entry) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    Beam::IO::SharedBuffer parameterBuffer;
    Beam::Serialization::BinarySender<Beam::IO::SharedBuffer> sender;
    sender.SetSink(Beam::Ref(parameterBuffer));
    try {
      sender.Shuttle(entry.GetSchema().GetParameters());
    } catch(const Beam::Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{
        "Unable to store schema parameters."});
    }
    Details::compliance_rule_entries row{entry.GetId(),
      entry.GetDirectoryEntry().m_id,
      static_cast<mysqlpp::sql_int_unsigned>(entry.GetDirectoryEntry().m_type),
      entry.GetState(), entry.GetSchema().GetName(),
      mysqlpp::sql_blob{parameterBuffer.GetData(), parameterBuffer.GetSize()}};
    query.replace(row);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{query.error()});
    }
  }

  inline void MySqlComplianceRuleDataStore::Delete(ComplianceRuleId id) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    query << "DELETE FROM compliance_rule_entries WHERE entry_id = " << id;
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{query.error()});
    }
  }

  inline void MySqlComplianceRuleDataStore::Store(
      const ComplianceRuleViolationRecord& violationRecord) {
    boost::lock_guard<Beam::Threading::Mutex> lock{m_mutex};
    auto query = m_databaseConnection.query();
    Details::compliance_rule_violation_records row{
      violationRecord.m_account.m_id, violationRecord.m_orderId,
      violationRecord.m_ruleId, violationRecord.m_schemaName,
      violationRecord.m_reason,
      Beam::MySql::ToMySqlTimestamp(violationRecord.m_timestamp)};
    query.insert(row);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{query.error()});
    }
  }

  inline void MySqlComplianceRuleDataStore::Open() {
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
        BOOST_THROW_EXCEPTION(Beam::IO::ConnectException{std::string{
          "Unable to connect to MySQL database - "} +
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

  inline void MySqlComplianceRuleDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlComplianceRuleDataStore::Shutdown() {
    m_databaseConnection.disconnect();
    m_openState.SetClosed();
  }

  inline ComplianceRuleEntry MySqlComplianceRuleDataStore::ConvertRow(
      const Details::compliance_rule_entries& row) {
    Beam::ServiceLocator::DirectoryEntry directoryEntry{
      static_cast<Beam::ServiceLocator::DirectoryEntry::Type>(
      row.directory_entry_type), row.directory_entry, ""};
    Beam::Serialization::BinaryReceiver<Beam::IO::SharedBuffer> receiver;
    Beam::IO::SharedBuffer schemaBuffer{row.schema_parameters.data(),
      row.schema_parameters.size()};
    receiver.SetSource(Beam::Ref(schemaBuffer));
    std::vector<ComplianceParameter> schemaParameters;
    try {
      receiver.Shuttle(schemaParameters);
    } catch(const Beam::Serialization::SerializationException&) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException{
        "Unable to load schema parameters."});
    }
    ComplianceRuleSchema schema{row.schema_name, std::move(schemaParameters)};
    ComplianceRuleEntry result{row.entry_id, directoryEntry,
      static_cast<ComplianceRuleEntry::State>(row.state), schema};
    return result;
  }
}
}

#endif
