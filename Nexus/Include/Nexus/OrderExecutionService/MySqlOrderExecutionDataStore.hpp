#ifndef NEXUS_MYSQLORDEREXECUTIONDATASTORE_HPP
#define NEXUS_MYSQLORDEREXECUTIONDATASTORE_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/MySql/DatabaseConnectionPool.hpp>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/OrderExecutionService/MySqlOrderExecutionDataStoreDetails.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class MySqlOrderExecutionDataStore
      \brief Stores Order execution data in a MySQL database.
   */
  class MySqlOrderExecutionDataStore : private boost::noncopyable {
    public:

      //! The function used to load DirectoryEntries.
      using AccountSourceFunction = Beam::KeyValueCache<unsigned int,
        Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex>::SourceFunction;

      //! Constructs a MySqlOrderExecutionDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
        \param accountSourceFunction The function used to load DirectoryEntries.
      */
      MySqlOrderExecutionDataStore(const Beam::Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password,
        const AccountSourceFunction& accountSourceFunction);

      //! Constructs a MySqlOrderExecutionDataStore.
      /*!
        \param address The IP address of the MySQL database to connect to.
        \param schema The name of the schema.
        \param username The username to connect as.
        \param password The password associated with the <i>username</i>.
      */
      MySqlOrderExecutionDataStore(const Beam::Network::IpAddress& address,
        const std::string& schema, const std::string& username,
        const std::string& password);

      ~MySqlOrderExecutionDataStore();

      AccountOrderSubmissionEntry::InitialSequences LoadInitialSequences(
        const Beam::ServiceLocator::DirectoryEntry& account);

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const std::vector<SequencedAccountOrderInfo>& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Store(const std::vector<SequencedAccountExecutionReport>&
        executionReports);

      void Open();

      void Close();

    private:
      template<typename T, typename Row>
      using DataStore = Beam::Queries::SqlDataStore<AccountQuery, T, Row,
        Queries::SqlTranslator, Details::SqlFunctor>;
      Beam::Network::IpAddress m_address;
      std::string m_schema;
      std::string m_username;
      std::string m_password;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_accountEntries;
      Beam::MySql::DatabaseConnectionPool m_readerDatabaseConnectionPool;
      Beam::Threading::Sync<mysqlpp::Connection> m_readerDatabaseConnection;
      Beam::Threading::Sync<mysqlpp::Connection> m_writerDatabaseConnection;
      Beam::Threading::ThreadPool m_readerThreadPool;
      DataStore<OrderInfo, Details::status_submissions>
        m_statusSubmissionsDataStore;
      DataStore<OrderInfo, Details::submissions> m_submissionsDataStore;
      DataStore<ExecutionReport, Details::execution_reports>
        m_executionReportsDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void OpenDatabaseConnection(mysqlpp::Connection& connection);
  };

  //! Makes an OrderExecutionDataStore replicated among multiple database
  //! instances.
  /*!
    \param config The list of MySQL instances to replicate across.
    \param accountSourceFunction The function used to load DirectoryEntries.
  */
  inline std::unique_ptr<ReplicatedOrderExecutionDataStore>
      MakeReplicatedMySqlOrderExecutionDataStore(
      const std::vector<Beam::MySql::MySqlConfig>& config,
      const MySqlOrderExecutionDataStore::AccountSourceFunction&
      accountSourceFunction) {
    auto& primaryConfig = config.front();
    auto primary = MakeVirtualOrderExecutionDataStore(
      std::make_unique<MySqlOrderExecutionDataStore>(
      primaryConfig.m_address, primaryConfig.m_schema, primaryConfig.m_username,
      primaryConfig.m_password, accountSourceFunction));
    std::vector<std::unique_ptr<VirtualOrderExecutionDataStore>>
      duplicateDataStores;
    for(auto i = std::size_t{1}; i != config.size(); ++i) {
      auto& duplicateConfig = config[i];
      auto duplicate = MakeVirtualOrderExecutionDataStore(
        std::make_unique<MySqlOrderExecutionDataStore>(
        duplicateConfig.m_address, duplicateConfig.m_schema,
        duplicateConfig.m_username, duplicateConfig.m_password,
        accountSourceFunction));
      duplicateDataStores.push_back(std::move(duplicate));
    }
    auto dataStore = std::make_unique<ReplicatedOrderExecutionDataStore>(
      std::move(primary), std::move(duplicateDataStores));
    return dataStore;
  }

  inline MySqlOrderExecutionDataStore::MySqlOrderExecutionDataStore(
      const Beam::Network::IpAddress& address, const std::string& schema,
      const std::string& username, const std::string& password,
      const AccountSourceFunction& accountSourceFunction)
      : m_address(address),
        m_schema(schema),
        m_username(username),
        m_password(password),
        m_accountEntries(accountSourceFunction),
        m_readerDatabaseConnection(false),
        m_writerDatabaseConnection(false),
        m_statusSubmissionsDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection), Beam::Ref(m_readerThreadPool)),
        m_submissionsDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection), Beam::Ref(m_readerThreadPool)),
        m_executionReportsDataStore(Beam::Ref(m_readerDatabaseConnectionPool),
          Beam::Ref(m_readerDatabaseConnection),
          Beam::Ref(m_writerDatabaseConnection),
          Beam::Ref(m_readerThreadPool)) {}

  inline MySqlOrderExecutionDataStore::MySqlOrderExecutionDataStore(
      const Beam::Network::IpAddress& address, const std::string& schema,
      const std::string& username, const std::string& password)
      : MySqlOrderExecutionDataStore(address, schema, username, password,
        [] (unsigned int id) {
          return Beam::ServiceLocator::DirectoryEntry(
            Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT, id, "");
        }) {}

  inline MySqlOrderExecutionDataStore::~MySqlOrderExecutionDataStore() {
    Close();
  }

  inline AccountOrderSubmissionEntry::InitialSequences
      MySqlOrderExecutionDataStore::LoadInitialSequences(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    AccountOrderSubmissionEntry::InitialSequences initialSequences;
    initialSequences.m_nextOrderInfoSequence =
      m_submissionsDataStore.LoadInitialSequence(account);
    initialSequences.m_nextExecutionReportSequence =
      m_executionReportsDataStore.LoadInitialSequence(account);
    return initialSequences;
  }

  inline std::vector<SequencedOrderRecord> MySqlOrderExecutionDataStore::
      LoadOrderSubmissions(const AccountQuery& query) {
    auto orderInfo =
      [&] {
        if(Details::HasLiveCheck(query.GetFilter())) {
          return m_statusSubmissionsDataStore.Load(query);
        } else {
          return m_submissionsDataStore.Load(query);
        }
      }();
    std::vector<SequencedOrderRecord> orderRecords;
    for(auto& order : orderInfo) {
      order->m_fields.m_account = m_accountEntries.Load(
        order->m_fields.m_account.m_id);
      order->m_submissionAccount = m_accountEntries.Load(
        order->m_submissionAccount.m_id);
      auto sequencedExecutionReports = m_executionReportsDataStore.Load(
        "order_id = " + boost::lexical_cast<std::string>(order->m_orderId));
      std::vector<ExecutionReport> executionReports;
      for(auto& executionReport : sequencedExecutionReports) {
        executionReports.push_back(std::move(*executionReport));
      }
      orderRecords.push_back(Beam::Queries::MakeSequencedValue(
        OrderRecord{std::move(*order), std::move(executionReports)},
        order.GetSequence()));
    }
    return orderRecords;
  }

  inline std::vector<SequencedExecutionReport> MySqlOrderExecutionDataStore::
      LoadExecutionReports(const AccountQuery& query) {
    return m_executionReportsDataStore.Load(query);
  }

  inline void MySqlOrderExecutionDataStore::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_submissionsDataStore.Store(orderInfo);
    Details::live_orders liveOrdersRow{(*orderInfo)->m_orderId};
    Beam::Threading::With(m_writerDatabaseConnection,
      [&] (mysqlpp::Connection& connection) {
        auto query = connection.query();
        query.insert(liveOrdersRow);
        query.execute();
      });
  }

  inline void MySqlOrderExecutionDataStore::Store(
      const std::vector<SequencedAccountOrderInfo>& orderInfo) {
    const auto MAX_ROW_WRITE = 500;
    m_submissionsDataStore.Store(orderInfo);
    if(orderInfo.empty()) {
      return;
    }
    std::vector<Details::live_orders> rows;
    std::transform(orderInfo.begin(), orderInfo.end(), std::back_inserter(rows),
      [] (auto& orderInfo) {
        return (*orderInfo)->m_orderId;
      });
    Beam::Threading::With(m_writerDatabaseConnection,
      [&] (mysqlpp::Connection& connection) {
        auto insertStart = rows.begin();
        while(insertStart != rows.end()) {
          auto query = connection.query();
          if(std::distance(insertStart, rows.end()) > MAX_ROW_WRITE) {
            query.insert(insertStart, insertStart + MAX_ROW_WRITE);
            insertStart = insertStart + MAX_ROW_WRITE;
          } else {
            query.insert(insertStart, rows.end());
            insertStart = rows.end();
          }
          query.execute();
        }
      });
  }

  inline void MySqlOrderExecutionDataStore::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_executionReportsDataStore.Store(executionReport);
    if(IsTerminal((*executionReport)->m_status)) {
      Details::live_orders liveOrdersRow{(*executionReport)->m_id};
      Beam::Threading::With(m_writerDatabaseConnection,
        [&] (mysqlpp::Connection& connection) {
          auto query = connection.query();
          query << "DELETE FROM live_orders WHERE order_id = " <<
            (*executionReport)->m_id;
          query.execute();
        });
    }
  }

  inline void MySqlOrderExecutionDataStore::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    m_executionReportsDataStore.Store(executionReports);
    std::string queryString = "DELETE FROM live_orders WHERE ";
    auto count = 0;
    for(auto& executionReport : executionReports) {
      if(IsTerminal((*executionReport)->m_status)) {
        if(count != 0) {
          queryString += "OR ";
        }
        queryString += "order_id = " +
          boost::lexical_cast<std::string>((*executionReport)->m_id);
        ++count;
      }
    }
    if(count != 0) {
      Beam::Threading::With(m_writerDatabaseConnection,
        [&] (mysqlpp::Connection& connection) {
          auto query = connection.query();
          query << queryString;
          query.execute();
        });
    }
  }

  inline void MySqlOrderExecutionDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Beam::Threading::With(m_readerDatabaseConnection,
        [&] (mysqlpp::Connection& connection) {
          OpenDatabaseConnection(connection);
          if(!Details::LoadTables(connection, m_schema)) {
            BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
              "Unable to load database tables."));
          }
        });
      Beam::Threading::With(m_writerDatabaseConnection,
        [&] (mysqlpp::Connection& connection) {
          OpenDatabaseConnection(connection);
          if(!Details::LoadTables(connection, m_schema)) {
            BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
              "Unable to load database tables."));
          }
        });
      for(std::size_t i = 0; i < boost::thread::hardware_concurrency(); ++i) {
        auto connection = std::make_unique<mysqlpp::Connection>(false);
        OpenDatabaseConnection(*connection);
        m_readerDatabaseConnectionPool.Add(std::move(connection));
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlOrderExecutionDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlOrderExecutionDataStore::OpenDatabaseConnection(
      mysqlpp::Connection& connection) {
    bool connectionResult = connection.set_option(
      new mysqlpp::ReconnectOption(true));
    if(!connectionResult) {
      BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(
        "Unable to set MySQL reconnect option."));
    }
    connectionResult = connection.connect(m_schema.c_str(),
      m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
      m_address.GetPort());
    if(!connectionResult) {
      BOOST_THROW_EXCEPTION(Beam::IO::ConnectException(std::string(
        "Unable to connect to MySQL database - ") + connection.error()));
    }
  }

  inline void MySqlOrderExecutionDataStore::Shutdown() {
    m_readerDatabaseConnectionPool.Close();
    Beam::Threading::With(m_writerDatabaseConnection,
      [] (mysqlpp::Connection& connection) {
        connection.disconnect();
      });
    Beam::Threading::With(m_readerDatabaseConnection,
      [] (mysqlpp::Connection& connection) {
        connection.disconnect();
      });
    m_openState.SetClosed();
  }
}
}

#endif
