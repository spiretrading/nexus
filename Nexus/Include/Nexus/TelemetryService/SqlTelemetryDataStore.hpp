#ifndef NEXUS_TELEMETRY_SERVICE_SQL_TELEMETRY_DATA_STORE_HPP
#define NEXUS_TELEMETRY_SERVICE_SQL_TELEMETRY_DATA_STORE_HPP
#include <functional>
#include <thread>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queries/SqlDataStore.hpp>
#include <Beam/Sql/DatabaseConnectionPool.hpp>
#include "Nexus/OrderExecutionService/SqlDefinitions.hpp"
#include "Nexus/Queries/SqlTranslator.hpp"
#include "Nexus/TelemetryService/TelemetryDataStore.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"
#include "Nexus/TelemetryService/SqlDefinitions.hpp"

namespace Nexus::TelemetryService {

  /**
   * Stores telemetry data in an SQL database.
   * @param <C> The type of SQL connection.
   */
  template<typename C>
  class SqlTelemetryDataStore {
    public:

      /** The type of SQL connection. */
      using Connection = C;

      /** The callable used to build SQL connections. */
      using ConnectionBuilder = std::function<Connection ()>;

      /**
       * Constructs an SqlTelemetryDataStore.
       * @param connectionBuilder The callable used to build SQL connections.
       */
      explicit SqlTelemetryDataStore(ConnectionBuilder connectionBuilder);

      ~SqlTelemetryDataStore();

      std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
        const AccountQuery& query);

      void Store(const SequencedAccountTelemetryEvent& event);

      void Store(const std::vector<SequencedAccountTelemetryEvent>& events);

      void Close();

    private:
      template<typename V, typename I>
      using DataStore = Beam::Queries::SqlDataStore<Connection, V, I,
        Queries::SqlTranslator>;
      Beam::DatabaseConnectionPool<Connection> m_readerPool;
      Beam::DatabaseConnectionPool<Connection> m_writerPool;
      DataStore<Viper::Row<TelemetryEvent>,
        Viper::Row<Beam::ServiceLocator::DirectoryEntry>>
        m_telemetryEventDataStore;
      Beam::IO::OpenState m_openState;

      SqlTelemetryDataStore(const SqlTelemetryDataStore&) = delete;
      SqlTelemetryDataStore& operator =(const SqlTelemetryDataStore&) = delete;
  };

  template<typename C>
  SqlTelemetryDataStore<C>::SqlTelemetryDataStore(
    ConnectionBuilder connectionBuilder)
    : m_readerPool(std::thread::hardware_concurrency(), [&] {
        auto connection = std::make_unique<Connection>(connectionBuilder());
        connection->open();
        return connection;
      }),
      m_writerPool(1, [&] {
        auto connection = std::make_unique<Connection>(connectionBuilder());
        connection->open();
        return connection;
      }),
      m_telemetryEventDataStore("telemetry_events", GetTelemetryEventRow(),
        OrderExecutionService::GetAccountRow(), Beam::Ref(m_readerPool),
        Beam::Ref(m_writerPool)) {}

  template<typename C>
  SqlTelemetryDataStore<C>::~SqlTelemetryDataStore() {
    Close();
  }

  template<typename C>
  std::vector<SequencedTelemetryEvent>
      SqlTelemetryDataStore<C>::LoadTelemetryEvents(const AccountQuery& query) {
    return m_telemetryEventDataStore.Load(query);
  }

  template<typename C>
  void SqlTelemetryDataStore<C>::Store(
      const SequencedAccountTelemetryEvent& event) {
    m_telemetryEventDataStore.Store(event);
  }

  template<typename C>
  void SqlTelemetryDataStore<C>::Store(
      const std::vector<SequencedAccountTelemetryEvent>& events) {
    m_telemetryEventDataStore.Store(events);
  }

  template<typename C>
  void SqlTelemetryDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_telemetryEventDataStore.Close();
    m_writerPool.Close();
    m_readerPool.Close();
    m_openState.Close();
  }
}

#endif
