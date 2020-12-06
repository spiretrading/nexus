#ifndef NEXUS_SQL_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_SQL_ORDER_EXECUTION_DATA_STORE_HPP
#include <functional>
#include <thread>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Stores Order execution data in an SQL database.
   * @param <C> The type of SQL connection to use.
   */
  template<typename C>
  class SqlOrderExecutionDataStore {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /** The callable used to build SQL connections. */
      using ConnectionBuilder = std::function<Connection ()>;

      /** The function used to load DirectoryEntries. */
      using AccountSourceFunction = Beam::KeyValueCache<unsigned int,
        Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex>::SourceFunction;

      /**
       * Constructs an SqlOrderExecutionDataStore.
       * @param connectionBuilder The callable used to build SQL connections.
       * @param accountSourceFunction The function used to load
       *        DirectoryEntries.
       */
      SqlOrderExecutionDataStore(ConnectionBuilder connectionBuilder,
        const AccountSourceFunction& accountSourceFunction);

      /**
       * Constructs a MySqlOrderExecutionDataStore.
       * @param connectionBuilder The callable used to build SQL connections.
       */
      SqlOrderExecutionDataStore(ConnectionBuilder connectionBuilder);

      ~SqlOrderExecutionDataStore();

      boost::optional<SequencedOrderRecord> LoadOrder(OrderId id);

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const std::vector<SequencedAccountOrderInfo>& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Store(const std::vector<SequencedAccountExecutionReport>&
        executionReports);

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Beam::Queries::SqlDataStore<Connection, V, I,
        Queries::SqlTranslator>;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_accountEntries;
      Beam::DatabaseConnectionPool<Connection> m_readerPool;
      Beam::DatabaseConnectionPool<Connection> m_writerPool;
      DataStore<Viper::Row<OrderInfo>,
        Viper::Row<Beam::ServiceLocator::DirectoryEntry>> m_submissionDataStore;
      DataStore<Viper::Row<OrderInfo>,
        Viper::Row<Beam::ServiceLocator::DirectoryEntry>>
        m_statusSubmissionDataStore;
      DataStore<Viper::Row<ExecutionReport>,
        Viper::Row<Beam::ServiceLocator::DirectoryEntry>>
        m_executionReportDataStore;
      Viper::Row<OrderId> m_liveOrdersRow;
      Beam::IO::OpenState m_openState;

      SqlOrderExecutionDataStore(const SqlOrderExecutionDataStore&) = delete;
      SqlOrderExecutionDataStore& operator =(
        const SqlOrderExecutionDataStore&) = delete;
      SequencedOrderRecord LoadRecord(SequencedOrderInfo info);
  };

  /**
   * Makes an OrderExecutionDataStore replicated among multiple database
   * instances.
   * @param connectionBuilders Builds the connections to the SQL database to
   *        replicate.
   * @param accountSourceFunction The function used to load DirectoryEntries.
   */
  template<typename Connection>
  auto MakeReplicatedMySqlOrderExecutionDataStore(
      const std::vector<std::function<Connection ()>>& connectionBuilders,
      const typename SqlOrderExecutionDataStore<
      Connection>::AccountSourceFunction& accountSourceFunction) {
    auto& primaryConnectionBuilder = connectionBuilders.front();
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
      : m_accountEntries(accountSourceFunction),
        m_readerPool(std::thread::hardware_concurrency(), [&] {
          auto connection = std::make_unique<Connection>(connectionBuilder());
          connection->open();
          return connection;
        }),
        m_writerPool(1, [&] {
          auto connection = std::make_unique<Connection>(connectionBuilder());
          connection->open();
          return connection;
        }),
        m_submissionDataStore("submissions", GetOrderInfoRow(), GetAccountRow(),
          Beam::Ref(m_readerPool), Beam::Ref(m_writerPool)),
        m_statusSubmissionDataStore("status_submissions", GetOrderInfoRow(),
          GetAccountRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Queries::SqlConnectionOption::NONE),
        m_executionReportDataStore("execution_reports", GetExecutionReportRow(),
          GetAccountRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool)) {
    m_liveOrdersRow = Viper::Row<OrderId>().
      add_column("order_id").
      set_primary_key("order_id");
    try {
      auto writerConnection = m_writerPool.Acquire();
      writerConnection->execute(Viper::create_if_not_exists(m_liveOrdersRow,
        "live_orders"));
      if(!writerConnection->has_table("status_submissions")) {
        writerConnection->execute("CREATE VIEW status_submissions AS "
          "SELECT submissions.*, IFNULL(live_orders.order_id, 0) != 0 AS "
          "is_live FROM submissions LEFT JOIN live_orders ON "
          "submissions.order_id = live_orders.order_id");
      }
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C>
  SqlOrderExecutionDataStore<C>::SqlOrderExecutionDataStore(
    ConnectionBuilder connectionBuilder)
    : SqlOrderExecutionDataStore(std::move(connectionBuilder),
      [] (unsigned int id) {
        return Beam::ServiceLocator::DirectoryEntry::MakeAccount(id, "");
      }) {}

  template<typename C>
  SqlOrderExecutionDataStore<C>::~SqlOrderExecutionDataStore() {
    Close();
  }

  template<typename C>
  boost::optional<SequencedOrderRecord>
      SqlOrderExecutionDataStore<C>::LoadOrder(OrderId id) {
    auto orders = m_submissionDataStore.Load(Viper::sym("order_id") == id);
    if(orders.empty()) {
      return boost::none;
    }
    return LoadRecord(std::move(orders.front()));
  }

  template<typename C>
  std::vector<SequencedOrderRecord> SqlOrderExecutionDataStore<C>::
      LoadOrderSubmissions(const AccountQuery& query) {
    auto orderInfo = [&] {
      if(HasLiveCheck(query.GetFilter())) {
        return m_statusSubmissionDataStore.Load(query);
      } else {
        return m_submissionDataStore.Load(query);
      }
    }();
    auto orderRecords = std::vector<SequencedOrderRecord>();
    for(auto& order : orderInfo) {
      orderRecords.push_back(LoadRecord(std::move(order)));
    }
    return orderRecords;
  }

  template<typename C>
  std::vector<SequencedExecutionReport> SqlOrderExecutionDataStore<C>::
      LoadExecutionReports(const AccountQuery& query) {
    return m_executionReportDataStore.Load(query);
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_submissionDataStore.Store(orderInfo);
    auto connection = m_writerPool.Acquire();
    connection->execute(Viper::insert(m_liveOrdersRow, "live_orders",
      &(*orderInfo)->m_orderId));
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
    auto connection = m_writerPool.Acquire();
    connection->execute(Viper::insert(m_liveOrdersRow, "live_orders",
      orderIds.begin(), orderIds.end()));
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_executionReportDataStore.Store(executionReport);
    if(IsTerminal((*executionReport)->m_status)) {
      auto connection = m_writerPool.Acquire();
      connection->execute(Viper::erase("live_orders",
        Viper::sym("order_id") == (*executionReport)->m_id));
    }
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const std::vector<SequencedAccountExecutionReport>& executionReports) {
    m_executionReportDataStore.Store(executionReports);
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
      auto connection = m_writerPool.Acquire();
      connection->execute(Viper::erase("live_orders", eraseCondition));
    }
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_executionReportDataStore.Close();
    m_statusSubmissionDataStore.Close();
    m_submissionDataStore.Close();
    m_writerPool.Close();
    m_readerPool.Close();
    m_openState.Close();
  }

  template<typename C>
  SequencedOrderRecord SqlOrderExecutionDataStore<C>::LoadRecord(
      SequencedOrderInfo info) {
    info->m_fields.m_account = m_accountEntries.Load(
      info->m_fields.m_account.m_id);
    info->m_submissionAccount = m_accountEntries.Load(
      info->m_submissionAccount.m_id);
    auto sequencedExecutionReports = m_executionReportDataStore.Load(
      Viper::sym("order_id") == info->m_orderId);
    auto executionReports = std::vector<ExecutionReport>();
    for(auto& executionReport : sequencedExecutionReports) {
      executionReports.push_back(std::move(*executionReport));
    }
    auto sequence = info.GetSequence();
    return Beam::Queries::SequencedValue(OrderRecord(std::move(*info),
      std::move(executionReports)), sequence);
  }
}

#endif
