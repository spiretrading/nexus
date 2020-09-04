#ifndef NEXUS_SQL_RISK_DATA_STORE_HPP
#define NEXUS_SQL_RISK_DATA_STORE_HPP
#include <memory>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/noncopyable.hpp>
#include <Viper/Viper.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/SqlDefinitions.hpp"

namespace Nexus::RiskService {

  /**
   * Implements a RiskDataStore backed by an SQL database.
   * @param <C> The SQL connection to use.
   */
  template<typename C>
  class SqlRiskDataStore : private boost::noncopyable {
    public:

      /** The SQL connection to use. */
      using Connection = C;

      /**
       * Constructs an SqlRiskDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlRiskDataStore(std::unique_ptr<Connection> connection);

      ~SqlRiskDataStore();

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);

      void Close();

    private:
      mutable Beam::Threading::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
  };

  template<typename C>
  SqlRiskDataStore<C>::SqlRiskDataStore(std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    m_openState.SetOpening();
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        GetInventoryEntriesRow(), "inventory_entries"));
      m_connection->execute(Viper::create_if_not_exists(
        GetInventorySequencesRow(), "inventory_sequences"));
      m_connection->execute(Viper::create_if_not_exists(
        GetInventoryExcludedOrdersRow(), "inventory_excluded_orders"));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  SqlRiskDataStore<C>::~SqlRiskDataStore() {
    Close();
  }

  template<typename C>
  InventorySnapshot SqlRiskDataStore<C>::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto snapshot = InventorySnapshot();
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, [&] {
      m_connection->execute(Viper::select(GetInventoryEntriesRow(),
        "inventory_entries", Viper::sym("account") == account.m_id,
        boost::make_function_output_iterator([&] (const auto& row) {
          snapshot.m_inventories.push_back(std::move(row.m_inventory));
        })));
      m_connection->execute(Viper::select(
        Viper::Row<Beam::Queries::Sequence>("sequence"), "inventory_sequences",
        Viper::sym("account") == account.m_id, &snapshot.m_sequence));
      m_connection->execute(Viper::select(GetInventoryExcludedOrdersRow(),
        "inventory_excluded_orders", Viper::sym("account") == account.m_id,
        boost::make_function_output_iterator([&] (const auto& row) {
          snapshot.m_excludedOrders.push_back(row.m_id);
        })));
    });
    return snapshot;
  }

  template<typename C>
  void SqlRiskDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    auto strippedSnapshot = Strip(snapshot);
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, [&] {
      m_connection->execute(Viper::erase("inventory_entries",
        Viper::sym("account") == account.m_id));
      m_connection->execute(Viper::erase("inventory_sequences",
        Viper::sym("account") == account.m_id));
      m_connection->execute(Viper::erase("inventory_excluded_orders",
        Viper::sym("account") == account.m_id));
      m_connection->execute(Viper::insert(GetInventoryEntriesRow(),
        "inventory_entries", boost::iterators::make_transform_iterator(
        strippedSnapshot.m_inventories.begin(),
        ConvertInventorySnapshotInventories(account)),
        boost::iterators::make_transform_iterator(
        strippedSnapshot.m_inventories.end(),
        ConvertInventorySnapshotInventories(account))));
      auto sequence = InventorySequence{account.m_id,
        strippedSnapshot.m_sequence};
      m_connection->execute(Viper::insert(GetInventorySequencesRow(),
        "inventory_sequences", &sequence));
      m_connection->execute(Viper::insert(GetInventoryExcludedOrdersRow(),
        "inventory_excluded_orders", boost::iterators::make_transform_iterator(
        strippedSnapshot.m_excludedOrders.begin(),
        ConvertInventoryExcludedOrders(account)),
        boost::iterators::make_transform_iterator(
        strippedSnapshot.m_excludedOrders.end(), ConvertInventoryExcludedOrders(
        account))));
    });
  }

  template<typename C>
  void SqlRiskDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlRiskDataStore<C>::Shutdown() {
    m_tasks.Break();
    m_tasks.Wait();
    m_connection->close();
    m_openState.SetClosed();
  }
}

#endif
