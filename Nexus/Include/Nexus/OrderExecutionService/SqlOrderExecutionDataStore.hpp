#ifndef NEXUS_SQL_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_SQL_ORDER_EXECUTION_DATA_STORE_HPP
#include <functional>
#include <thread>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus::OrderExecutionService {

  /** Stores Order execution data in an SQL database.
      \tparam C The type of SQL connection to use.
   */
  template<typename C>
  class SqlOrderExecutionDataStore : private boost::noncopyable {
    public:

      //! The type of SQL connection.
      using Connection = C;

      //! The callable used to build SQL connections.
      using ConnectionBuilder = std::function<Connection ()>;

      //! The function used to load DirectoryEntries.
      using AccountSourceFunction = Beam::KeyValueCache<unsigned int,
        Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex>::SourceFunction;

      //! Constructs an SqlOrderExecutionDataStore.
      /*!
        \param connectionBuilder The callable used to build SQL connections.
        \param accountSourceFunction The function used to load DirectoryEntries.
      */
      SqlOrderExecutionDataStore(ConnectionBuilder connectionBuilder,
        const AccountSourceFunction& accountSourceFunction);

      //! Constructs a MySqlOrderExecutionDataStore.
      /*!
        \param connectionBuilder The callable used to build SQL connections.
      */
      SqlOrderExecutionDataStore(ConnectionBuilder connectionBuilder);

      ~SqlOrderExecutionDataStore();

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
      template<typename V, typename I>
      using DataStore = Beam::Queries::SqlDataStore<Connection, V, I,
        Queries::SqlTranslator>;
      ConnectionBuilder m_connectionBuilder;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_accountEntries;
      Beam::DatabaseConnectionPool<Connection> m_connectionPool;
      Beam::Threading::Sync<Connection, Beam::Threading::Mutex>
        m_writeConnection;
      Beam::Threading::ThreadPool m_threadPool;
      DataStore<Viper::Row<OrderRecord>, Viper::Row<unsigned int>>
        m_submissionDataStore;
      DataStore<Viper::Row<OrderRecord>, Viper::Row<unsigned int>>
        m_statusSubmissionDataStore;
      DataStore<Viper::Row<ExecutionReport>, Viper::Row<unsigned int>>
        m_executionReportDataStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes an OrderExecutionDataStore replicated among multiple database
  //! instances.
  /*!
    \param connectionBuilders Builds the connections to the SQL database to
           replicate.
    \param accountSourceFunction The function used to load DirectoryEntries.
  */
  template<typename Connection>
  auto MakeReplicatedMySqlOrderExecutionDataStore(
      const std::vector<std::function<Connection ()>>& connectionBuilders,
      const typename SqlOrderExecutionDataStore<
      Connection>::AccountSourceFunction& accountSourceFunction) {
    auto& primaryConnectionBuilder = config.front();
    auto primary = MakeVirtualOrderExecutionDataStore(
      std::make_unique<SqlOrderExecutionDataStore<Connection>>(
      primaryConnectionBuilder, accountSourceFunction));
    auto duplicateDataStores =
      std::vector<std::unique_ptr<VirtualOrderExecutionDataStore>>();
    for(auto i = std::size_t(1); i < connectionBuilders.size(); ++i) {
      auto duplicate = MakeVirtualOrderExecutionDataStore(
        std::make_unique<SqlOrderExecutionDataStore<Connection>>(
        connectionBuilders[i], accountSourceFunction));
      duplicateDataStores.push_back(std::move(duplicate));
    }
    auto dataStore = std::make_unique<ReplicatedOrderExecutionDataStore>(
      std::move(primary), std::move(duplicateDataStores));
    return dataStore;
  }

  template<typename C>
  SqlOrderExecutionDataStore<C>::SqlOrderExecutionDataStore(
      ConnectionBuilder connectionBuilder,
      const AccountSourceFunction& accountSourceFunction)
      : m_connectionBuilder(std::move(connectionBuilder)) {}

  template<typename C>
  SqlOrderExecutionDataStore<C>::SqlOrderExecutionDataStore(
      ConnectionBuilder connectionBuilder)
      : MySqlOrderExecutionDataStore(std::move(connectionBuilder),
        [] (unsigned int id) {
          return Beam::ServiceLocator::DirectoryEntry::MakeAccount(id, "");
        }} {}

  template<typename C>
  SqlOrderExecutionDataStore<C>::~SqlOrderExecutionDataStore() {
    Close();
  }

  template<typename C>
  std::vector<SequencedOrderRecord> SqlOrderExecutionDataStore<C>::
      LoadOrderSubmissions(const AccountQuery& query) {
    auto orderInfo =
      [&] {
        if(HasLiveCheck(query.GetFilter())) {
          return m_statusSubmissionDataStore.Load(query);
        } else {
          return m_submissionDataStore.Load(query);
        }
      }();
    auto orderRecords = std::vector<SequencedOrderRecord>();
    for(auto& order : orderInfo) {
      order->m_fields.m_account = m_accountEntries.Load(
        order->m_fields.m_account.m_id);
      order->m_submissionAccount = m_accountEntries.Load(
        order->m_submissionAccount.m_id);
      auto sequencedExecutionReports = m_executionReportsDataStore.Load(
        Viper::sym("order_id") == order->m_orderId);
      auto executionReports = std::vector<ExecutionReport>();
      for(auto& executionReport : sequencedExecutionReports) {
        executionReports.push_back(std::move(*executionReport));
      }
      orderRecords.push_back(Beam::Queries::MakeSequencedValue(
        OrderRecord(std::move(*order), std::move(executionReports)),
        order.GetSequence()));
    }
    return orderRecords;
  }

  template<typename C>
  std::vector<SequencedExecutionReport> SqlOrderExecutionDataStore<C>::
      LoadExecutionReports(const AccountQuery& query) {
    return m_executionReportsDataStore.Load(query);
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_submissionsDataStore.Store(orderInfo);
    Beam::Threading::With(m_writeConnection,
      [&] (auto& connection) {
        connection.execute(Viper::insert("live_orders",
          &(*orderInfo)->m_orderId));
      });
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const std::vector<SequencedAccountOrderInfo>& orderInfo) {
    if(orderInfo.empty()) {
      return;
    }
    m_submissionDataStore.Store(orderInfo);
    auto orderIds = std::vector<OrderId>();
    std::transform(orderInfo.begin(), orderInfo.end(),
      std::back_inserter(orderIds),
      [] (auto& orderInfo) {
        return (*orderInfo)->m_orderId;
      });
    Beam::Threading::With(m_writeConnection,
      [&] (auto& connection) {
        connection.execute(Viper::insert("live_orders", orderIds.begin(),
          orderIds.end()));
      });
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_executionReportsDataStore.Store(executionReport);
    if(IsTerminal((*executionReport)->m_status)) {
      Beam::Threading::With(m_writeConnection,
        [&] (auto& connection) {
          connection.execute(Viper::erase("live_orders",
            Viper::sym("order_id") == (*executionReport)->m_id));
        });
    }
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    m_executionReportsDataStore.Store(executionReports);
    auto eraseCondition = Viper::literal(false);
    auto hasErase = false;
    for(auto& executionReport : executionReports) {
      if(IsTerminal((*executionReport)->m_status)) {
        hasErase = true;
        eraseCondition = eraseCondition || Viper::sym("order_id") ==
          (*executionReport)->m_id;
      }
    }
    if(hasErase) {
      Beam::Threading::With(m_writeConnection,
        [&] (auto& connection) {
          connection.execute(Viper::erase("live_orders", eraseCondition));
        });
    }
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Beam::Threading::With(m_writeConnection,
        [&] (auto& connection) {
          connection.open();
        });
      for(auto i = std::size_t(0);
          i <= std::thread::hardware_concurrency(); ++i) {
        auto connection = std::make_unique<Connection>(m_connectionBuilder());
        connection->open();
        m_connectionPool.Add(std::move(connection));
      }
      m_submissionDataStore.Open();
      m_statusSubmissionDataStore.Open();
      m_executionReportDataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Shutdown() {
    m_executionReportDataStore.Close();
    m_statusSubmissionDataStore.Close();
    m_submissionDataStore.Close();
    m_connectionPool.Close();
    Beam::Threading::With(m_writeConnection,
      [] (auto& connection) {
        connection.close();
      });
    m_openState.SetClosed();
  }
}

#endif
