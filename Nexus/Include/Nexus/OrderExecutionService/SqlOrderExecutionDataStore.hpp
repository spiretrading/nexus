#ifndef NEXUS_SQL_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_SQL_ORDER_EXECUTION_DATA_STORE_HPP
#include <functional>
#include <thread>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"

namespace Nexus {

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
      using AccountSource = Beam::KeyValueCache<
        unsigned int, Beam::DirectoryEntry, Beam::Mutex>::Source;

      /**
       * Constructs an SqlOrderExecutionDataStore.
       * @param connection_builder The callable used to build SQL connections.
       * @param account_source_function The function used to load
       *        DirectoryEntries.
       */
      SqlOrderExecutionDataStore(ConnectionBuilder connection_builder,
        const AccountSource& account_source_function);

      /**
       * Constructs a MySqlOrderExecutionDataStore.
       * @param connection_builder The callable used to build SQL connections.
       */
      explicit SqlOrderExecutionDataStore(ConnectionBuilder connection_builder);

      ~SqlOrderExecutionDataStore();

      boost::optional<SequencedAccountOrderRecord>
        load_order_record(OrderId id);
      std::vector<SequencedOrderRecord>
        load_order_records(const AccountQuery& query);
      void store(const SequencedAccountOrderInfo& info);
      void store(const std::vector<SequencedAccountOrderInfo>& info);
      std::vector<SequencedExecutionReport>
        load_execution_reports(const AccountQuery& query);
      void store(const SequencedAccountExecutionReport& report);
      void store(const std::vector<SequencedAccountExecutionReport>& reports);
      void close();

    private:
      template<bool IsLive>
      struct Translator : SqlTranslator {
        using SqlTranslator::SqlTranslator;

        void visit(const Beam::MemberAccessExpression& expression) override;
      };
      template<typename V, typename I>
      using DataStore = Beam::SqlDataStore<Connection, V, I, SqlTranslator>;
      template<bool IsLive>
      using IsLiveDataStore = Beam::SqlDataStore<
        Connection, Viper::Row<OrderInfo>, Viper::Row<Beam::DirectoryEntry>,
        Translator<IsLive>>;
      Beam::KeyValueCache<unsigned int, Beam::DirectoryEntry, Beam::Mutex>
        m_account_entries;
      Beam::DatabaseConnectionPool<Connection> m_reader_pool;
      Beam::DatabaseConnectionPool<Connection> m_writer_pool;
      DataStore<Viper::Row<OrderInfo>, Viper::Row<Beam::DirectoryEntry>>
        m_submissions_data_store;
      IsLiveDataStore<true> m_live_submissions_data_store;
      IsLiveDataStore<false> m_terminal_submissions_data_store;
      DataStore<Viper::Row<ExecutionReport>, Viper::Row<Beam::DirectoryEntry>>
        m_execution_reports_data_store;
      Viper::Row<OrderId> m_live_orders_row;
      Beam::OpenState m_open_state;

      SqlOrderExecutionDataStore(const SqlOrderExecutionDataStore&) = delete;
      SqlOrderExecutionDataStore& operator =(
        const SqlOrderExecutionDataStore&) = delete;
      SequencedOrderRecord load(SequencedOrderInfo info);
  };

  /**
   * Makes an OrderExecutionDataStore replicated among multiple database
   * instances.
   * @param connection_builders Constructs the connections to the SQL database
   *        to replicate.
   * @param account_source_function The function used to load DirectoryEntries.
   */
  template<typename Connection>
  auto make_replicated_sql_order_execution_data_store(
      const std::vector<std::function<Connection ()>>& connection_builders,
      const typename SqlOrderExecutionDataStore<Connection>::AccountSource&
        account_source_function) {
    auto& primary_connection_builder = connection_builders.front();
    auto primary = OrderExecutionDataStore(
      std::in_place_type<SqlOrderExecutionDataStore<Connection>>,
      primary_connection_builder, account_source_function);
    auto duplicate_data_stores = std::vector<OrderExecutionDataStore>();
    for(auto i = std::size_t(1); i < connection_builders.size(); ++i) {
      auto duplicate = OrderExecutionDataStore(
        std::in_place_type<SqlOrderExecutionDataStore<Connection>>,
        connection_builders[i], account_source_function);
      duplicate_data_stores.push_back(std::move(duplicate));
    }
    auto data_store = std::make_unique<ReplicatedOrderExecutionDataStore>(
      std::move(primary), std::move(duplicate_data_stores));
    return data_store;
  }

  template<typename C>
  SqlOrderExecutionDataStore<C>::SqlOrderExecutionDataStore(
      ConnectionBuilder connection_builder,
      const AccountSource& account_source_function)
      : m_account_entries(account_source_function),
        m_reader_pool(std::thread::hardware_concurrency(), [&] {
          auto connection = std::make_unique<Connection>(connection_builder());
          connection->open();
          return connection;
        }),
        m_writer_pool(1, [&] {
          auto connection = std::make_unique<Connection>(connection_builder());
          connection->open();
          return connection;
        }),
        m_submissions_data_store("submissions", get_order_info_row(),
          get_account_row(), Beam::Ref(m_reader_pool),
          Beam::Ref(m_writer_pool)),
        m_live_submissions_data_store("live_submissions", get_order_info_row(),
          get_account_row(), Beam::Ref(m_reader_pool), Beam::Ref(m_writer_pool),
          Beam::SqlConnectionOption::NONE),
        m_terminal_submissions_data_store("terminal_submissions",
          get_order_info_row(), get_account_row(), Beam::Ref(m_reader_pool),
          Beam::Ref(m_writer_pool), Beam::SqlConnectionOption::NONE),
        m_execution_reports_data_store("execution_reports",
          get_execution_report_row(), get_account_row(),
          Beam::Ref(m_reader_pool), Beam::Ref(m_writer_pool)) {
    m_live_orders_row = Viper::Row<OrderId>().
      add_column("order_id").
      set_primary_key("order_id");
    try {
      auto writer_connection = m_writer_pool.load();
      writer_connection->execute(
        Viper::create_if_not_exists(m_live_orders_row, "live_orders"));
      if(!writer_connection->has_table("live_submissions")) {
        writer_connection->execute("CREATE VIEW live_submissions AS "
          "SELECT * FROM submissions WHERE order_id IN ("
          "SELECT order_id FROM live_orders)");
      }
      if(!writer_connection->has_table("terminal_submissions")) {
        writer_connection->execute("CREATE VIEW terminal_submissions AS "
          "SELECT * FROM submissions WHERE order_id NOT IN ("
          "SELECT order_id FROM live_orders)");
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlOrderExecutionDataStore<C>::SqlOrderExecutionDataStore(
    ConnectionBuilder connection_builder)
    : SqlOrderExecutionDataStore(std::move(connection_builder),
        [] (unsigned int id) {
          return Beam::DirectoryEntry::make_account(id, "");
        }) {}

  template<typename C>
  SqlOrderExecutionDataStore<C>::~SqlOrderExecutionDataStore() {
    close();
  }

  template<typename C>
  boost::optional<SequencedAccountOrderRecord>
      SqlOrderExecutionDataStore<C>::load_order_record(OrderId id) {
    auto orders = m_submissions_data_store.load(Viper::sym("order_id") == id);
    if(orders.empty()) {
      return boost::none;
    }
    auto record = load(std::move(orders.front()));
    return Beam::SequencedValue(Beam::IndexedValue(std::move(*record),
      record->m_info.m_fields.m_account), record.get_sequence());
  }

  template<typename C>
  std::vector<SequencedOrderRecord>
      SqlOrderExecutionDataStore<C>::load_order_records(
        const AccountQuery& query) {
    auto info = [&] {
      if(has_live_check(query.get_filter())) {
        auto live_submissions = m_live_submissions_data_store.load(query);
        auto terminal_submissions =
          m_terminal_submissions_data_store.load(query);
        auto submissions = std::vector<SequencedOrderInfo>();
        std::ranges::set_union(live_submissions, terminal_submissions,
          std::back_inserter(submissions), Beam::SequenceComparator());
        return submissions;
      }
      return m_submissions_data_store.load(query);
    }();
    auto records = std::vector<SequencedOrderRecord>();
    for(auto& order : info) {
      records.push_back(load(std::move(order)));
    }
    return records;
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::store(
      const SequencedAccountOrderInfo& info) {
    m_submissions_data_store.store(info);
    auto connection = m_writer_pool.load();
    connection->execute(
      Viper::insert(m_live_orders_row, "live_orders", &(*info)->m_id));
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    if(info.empty()) {
      return;
    }
    m_submissions_data_store.store(info);
    auto ids = std::vector<OrderId>();
    std::transform(info.begin(), info.end(), std::back_inserter(ids),
      [] (auto& i) {
        return (*i)->m_id;
      });
    auto connection = m_writer_pool.load();
    connection->execute(Viper::insert(
      m_live_orders_row, "live_orders", ids.begin(), ids.end()));
  }

  template<typename C>
  std::vector<SequencedExecutionReport>
      SqlOrderExecutionDataStore<C>::load_execution_reports(
        const AccountQuery& query) {
    return m_execution_reports_data_store.load(query);
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::store(
      const SequencedAccountExecutionReport& report) {
    m_execution_reports_data_store.store(report);
    if(is_terminal((*report)->m_status)) {
      auto connection = m_writer_pool.load();
      connection->execute(
        Viper::erase("live_orders", Viper::sym("order_id") == (*report)->m_id));
    }
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    m_execution_reports_data_store.store(reports);
    auto erase_condition = Viper::literal(false);
    auto has_erase = false;
    for(auto& report : reports) {
      if(is_terminal((*report)->m_status)) {
        has_erase = true;
        erase_condition =
          erase_condition || Viper::sym("order_id") == (*report)->m_id;
      }
    }
    if(has_erase) {
      auto connection = m_writer_pool.load();
      connection->execute(Viper::erase("live_orders", erase_condition));
    }
  }

  template<typename C>
  void SqlOrderExecutionDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_execution_reports_data_store.close();
    m_terminal_submissions_data_store.close();
    m_live_submissions_data_store.close();
    m_submissions_data_store.close();
    m_writer_pool.close();
    m_reader_pool.close();
    m_open_state.close();
  }

  template<typename C>
  SequencedOrderRecord
      SqlOrderExecutionDataStore<C>::load(SequencedOrderInfo info) {
    info->m_fields.m_account =
      m_account_entries.load(info->m_fields.m_account.m_id);
    info->m_submission_account =
      m_account_entries.load(info->m_submission_account.m_id);
    auto sequenced_reports = m_execution_reports_data_store.load(
      Viper::sym("order_id") == info->m_id);
    auto reports = std::vector<ExecutionReport>();
    for(auto& report : sequenced_reports) {
      reports.push_back(std::move(*report));
    }
    auto sequence = info.get_sequence();
    return Beam::SequencedValue(
      OrderRecord(std::move(*info), std::move(reports)), sequence);
  }

  template<typename C>
  template<bool IsLive>
  void SqlOrderExecutionDataStore<C>::Translator<IsLive>::visit(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_expression().get_type() == typeid(OrderInfo) &&
        expression.get_name() == "is_live") {
      get_translation() = Viper::literal(IsLive);
    } else {
      SqlTranslator::visit(expression);
    }
  }
}

#endif
