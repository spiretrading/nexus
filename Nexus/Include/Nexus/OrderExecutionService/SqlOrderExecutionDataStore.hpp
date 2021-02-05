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
      explicit SqlOrderExecutionDataStore(ConnectionBuilder connectionBuilder);

      ~SqlOrderExecutionDataStore();

      boost::optional<SequencedAccountOrderRecord> LoadOrder(OrderId id);

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
      template<bool IsLive>
      struct Translator : Queries::SqlTranslator {
        using Queries::SqlTranslator::SqlTranslator;

        void Visit(
          const Beam::Queries::MemberAccessExpression& expression) override;
      };
      template<typename V, typename I>
      using DataStore =
        Beam::Queries::SqlDataStore<Connection, V, I, Queries::SqlTranslator>;
      template<bool IsLive>
      using IsLiveDataStore = Beam::Queries::SqlDataStore<Connection,
        Viper::Row<OrderInfo>, Viper::Row<Beam::ServiceLocator::DirectoryEntry>,
        Translator<IsLive>>;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_accountEntries;
      Beam::DatabaseConnectionPool<Connection> m_readerPool;
      Beam::DatabaseConnectionPool<Connection> m_writerPool;
      DataStore<Viper::Row<OrderInfo>,
        Viper::Row<Beam::ServiceLocator::DirectoryEntry>>
          m_submissionsDataStore;
      IsLiveDataStore<true> m_liveSubmissionsDataStore;
      IsLiveDataStore<false> m_terminalSubmissionsDataStore;
      DataStore<Viper::Row<ExecutionReport>,
        Viper::Row<Beam::ServiceLocator::DirectoryEntry>>
          m_executionReportsDataStore;
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
   * @param connectionBuilders Constructs the connections to the SQL database to
   *        replicate.
   * @param accountSourceFunction The function used to load DirectoryEntries.
   */
  template<typename Connection>
  auto MakeReplicatedMySqlOrderExecutionDataStore(
      const std::vector<std::function<Connection ()>>& connectionBuilders,
      const typename SqlOrderExecutionDataStore<
      Connection>::AccountSourceFunction& accountSourceFunction) {
    auto& primaryConnectionBuilder = connectionBuilders.front();
    auto primary = OrderExecutionDataStoreBox(
      std::in_place_type<SqlOrderExecutionDataStore<Connection>>,
      primaryConnectionBuilder, accountSourceFunction);
    auto duplicateDataStores = std::vector<OrderExecutionDataStoreBox>();
    for(auto i = std::size_t(1); i < connectionBuilders.size(); ++i) {
      auto duplicate = OrderExecutionDataStoreBox(
        std::in_place_type<SqlOrderExecutionDataStore<Connection>>,
        connectionBuilders[i], accountSourceFunction);
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
        m_submissionsDataStore("submissions", GetOrderInfoRow(),
          GetAccountRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool)),
        m_liveSubmissionsDataStore("live_submissions", GetOrderInfoRow(),
          GetAccountRow(), Beam::Ref(m_readerPool), Beam::Ref(m_writerPool),
          Beam::Queries::SqlConnectionOption::NONE),
        m_terminalSubmissionsDataStore("terminal_submissions",
          GetOrderInfoRow(), GetAccountRow(), Beam::Ref(m_readerPool),
          Beam::Ref(m_writerPool), Beam::Queries::SqlConnectionOption::NONE),
        m_executionReportsDataStore("execution_reports",
          GetExecutionReportRow(), GetAccountRow(), Beam::Ref(m_readerPool),
          Beam::Ref(m_writerPool)) {
    m_liveOrdersRow = Viper::Row<OrderId>().
      add_column("order_id").
      set_primary_key("order_id");
    try {
      auto writerConnection = m_writerPool.Acquire();
      writerConnection->execute(Viper::create_if_not_exists(m_liveOrdersRow,
        "live_orders"));
      if(!writerConnection->has_table("live_submissions")) {
        writerConnection->execute("CREATE VIEW live_submissions AS "
          "SELECT * FROM submissions WHERE order_id IN ("
          "SELECT order_id FROM live_orders)");
      }
      if(!writerConnection->has_table("terminal_submissions")) {
        writerConnection->execute("CREATE VIEW terminal_submissions AS "
          "SELECT * FROM submissions WHERE order_id NOT IN ("
          "SELECT order_id FROM live_orders)");
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
  boost::optional<SequencedAccountOrderRecord>
      SqlOrderExecutionDataStore<C>::LoadOrder(OrderId id) {
    auto orders = m_submissionsDataStore.Load(Viper::sym("order_id") == id);
    if(orders.empty()) {
      return boost::none;
    }
    auto record = LoadRecord(std::move(orders.front()));
    return Beam::Queries::SequencedValue(Beam::Queries::IndexedValue(
      std::move(*record), record->m_info.m_fields.m_account),
      record.GetSequence());
  }

  template<typename C>
  std::vector<SequencedOrderRecord> SqlOrderExecutionDataStore<C>::
      LoadOrderSubmissions(const AccountQuery& query) {
    auto orderInfo = [&] {
      if(HasLiveCheck(query.GetFilter())) {
        auto liveSubmissions = m_liveSubmissionsDataStore.Load(query);
        auto terminalSubmissions = m_terminalSubmissionsDataStore.Load(query);
        auto submissions = std::vector<SequencedOrderInfo>();
        Beam::MergeWithoutDuplicates(liveSubmissions.begin(),
          liveSubmissions.end(), terminalSubmissions.begin(),
          terminalSubmissions.end(), std::back_inserter(submissions),
          Beam::Queries::SequenceComparator());
        return submissions;
      } else {
        return m_submissionsDataStore.Load(query);
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
    return m_executionReportsDataStore.Load(query);
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_submissionsDataStore.Store(orderInfo);
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
    m_submissionsDataStore.Store(orderInfo);
    auto orderIds = std::vector<OrderId>();
    std::transform(orderInfo.begin(), orderInfo.end(),
      std::back_inserter(orderIds), [] (auto& orderInfo) {
        return (*orderInfo)->m_orderId;
      });
    auto connection = m_writerPool.Acquire();
    connection->execute(Viper::insert(m_liveOrdersRow, "live_orders",
      orderIds.begin(), orderIds.end()));
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_executionReportsDataStore.Store(executionReport);
    if(IsTerminal((*executionReport)->m_status)) {
      auto connection = m_writerPool.Acquire();
      connection->execute(Viper::erase("live_orders",
        Viper::sym("order_id") == (*executionReport)->m_id));
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
        eraseCondition = eraseCondition ||
          Viper::sym("order_id") == (*executionReport)->m_id;
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
    m_executionReportsDataStore.Close();
    m_terminalSubmissionsDataStore.Close();
    m_liveSubmissionsDataStore.Close();
    m_submissionsDataStore.Close();
    m_writerPool.Close();
    m_readerPool.Close();
    m_openState.Close();
  }

  template<typename C>
  SequencedOrderRecord SqlOrderExecutionDataStore<C>::LoadRecord(
      SequencedOrderInfo info) {
    info->m_fields.m_account =
      m_accountEntries.Load(info->m_fields.m_account.m_id);
    info->m_submissionAccount =
      m_accountEntries.Load(info->m_submissionAccount.m_id);
    auto sequencedExecutionReports = m_executionReportsDataStore.Load(
      Viper::sym("order_id") == info->m_orderId);
    auto executionReports = std::vector<ExecutionReport>();
    for(auto& executionReport : sequencedExecutionReports) {
      executionReports.push_back(std::move(*executionReport));
    }
    auto sequence = info.GetSequence();
    return Beam::Queries::SequencedValue(
      OrderRecord(std::move(*info), std::move(executionReports)), sequence);
  }

  template<typename C>
  template<bool IsLive>
  void SqlOrderExecutionDataStore<C>::Translator<IsLive>::Visit(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetExpression()->GetType() == Queries::OrderInfoType() &&
        expression.GetName() == "is_live") {
      GetTranslation() = Viper::literal(IsLive);
    } else {
      Queries::SqlTranslator::Visit(expression);
    }
  }
}

#endif
