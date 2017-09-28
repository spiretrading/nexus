#ifndef NEXUS_MYSQLADMINISTRATIONDATASTORE_HPP
#define NEXUS_MYSQLADMINISTRATIONDATASTORE_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/MySql/PosixTimeToMySqlDateTime.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Threading/Mutex.hpp>
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

      //! Constructs a MySqlAdministrationDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlAdministrationDataStore(const Beam::Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password);

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
      mysqlpp::Connection m_databaseConnection;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline MySqlAdministrationDataStore::MySqlAdministrationDataStore(
      const Beam::Network::IpAddress& address, const std::string& schema,
      const std::string& username, const std::string& password)
      : m_address{address},
        m_schema{schema},
        m_username{username},
        m_password{password},
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
        Beam::ServiceLocator::DirectoryEntry account{
          Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT, row.account, ""};
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
    Details::SqlInsert::account_identities entryRow{account.m_id,
      identity.m_firstName, identity.m_lastName, identity.m_emailAddress,
      identity.m_addressLineOne, identity.m_addressLineTwo,
      identity.m_addressLineThree, identity.m_city, identity.m_province,
      identity.m_country, identity.m_userNotes, ""};
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
        Beam::ServiceLocator::DirectoryEntry account{
          Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT, row.account, ""};
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
    Details::SqlInsert::risk_parameters entryRow{account.m_id,
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
        Beam::ServiceLocator::DirectoryEntry account{
          Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT, row.account, ""};
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
    Details::SqlInsert::risk_states entryRow{account.m_id,
      static_cast<int>(riskState.m_type),
      Beam::MySql::ToDateTime(riskState.m_expiry)};
    query.replace(entryRow);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException{query.error()});
    }
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
