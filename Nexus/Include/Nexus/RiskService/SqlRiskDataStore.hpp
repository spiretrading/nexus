#ifndef NEXUS_SQL_RISK_DATA_STORE_HPP
#define NEXUS_SQL_RISK_DATA_STORE_HPP
#include <memory>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/iterator/function_output_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <Viper/Viper.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskService/SqlDefinitions.hpp"

namespace Nexus {

  /**
   * Implements a RiskDataStore backed by an SQL database.
   * @param <C> The SQL connection to use.
   */
  template<typename C>
  class SqlRiskDataStore {
    public:

      /** The SQL connection to use. */
      using Connection = C;

      /**
       * Constructs an SqlRiskDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlRiskDataStore(std::unique_ptr<Connection> connection);

      ~SqlRiskDataStore();
      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account,
        const InventorySnapshot& snapshot);
      void close();

    private:
      mutable Beam::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::OpenState m_open_state;

      SqlRiskDataStore(const SqlRiskDataStore&) = delete;
      SqlRiskDataStore& operator =(const SqlRiskDataStore&) = delete;
  };

  template<typename C>
  SqlRiskDataStore<C>::SqlRiskDataStore(std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        get_inventory_entries_row(), "inventory_entries"));
      m_connection->execute(Viper::create_if_not_exists(
        get_inventory_sequences_row(), "inventory_sequences"));
      m_connection->execute(Viper::create_if_not_exists(
        get_inventory_excluded_orders_row(), "inventory_excluded_orders"));
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C>
  SqlRiskDataStore<C>::~SqlRiskDataStore() {
    close();
  }

  template<typename C>
  InventorySnapshot SqlRiskDataStore<C>::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    auto snapshot = InventorySnapshot();
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, [&] {
      m_connection->execute(Viper::select(get_inventory_entries_row(),
        "inventory_entries", Viper::sym("account") == account.m_id,
        boost::make_function_output_iterator([&] (const auto& row) {
          snapshot.m_inventories.push_back(std::move(row.m_inventory));
        })));
      m_connection->execute(Viper::select(
        Viper::Row<Beam::Sequence>("sequence"), "inventory_sequences",
        Viper::sym("account") == account.m_id, &snapshot.m_sequence));
      m_connection->execute(Viper::select(get_inventory_excluded_orders_row(),
        "inventory_excluded_orders", Viper::sym("account") == account.m_id,
        boost::make_function_output_iterator([&] (const auto& row) {
          snapshot.m_excluded_orders.push_back(row.m_id);
        })));
    });
    return snapshot;
  }

  template<typename C>
  void SqlRiskDataStore<C>::store(
      const Beam::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    auto stripped_snapshot = strip(snapshot);
    auto lock = std::lock_guard(m_mutex);
    Viper::transaction(*m_connection, [&] {
      m_connection->execute(Viper::erase(
        "inventory_entries", Viper::sym("account") == account.m_id));
      m_connection->execute(Viper::erase(
        "inventory_sequences", Viper::sym("account") == account.m_id));
      m_connection->execute(Viper::erase(
        "inventory_excluded_orders", Viper::sym("account") == account.m_id));
      m_connection->execute(Viper::insert(get_inventory_entries_row(),
        "inventory_entries", boost::iterators::make_transform_iterator(
          stripped_snapshot.m_inventories.begin(),
          convert_inventory_snapshot_inventories(account)),
        boost::iterators::make_transform_iterator(
          stripped_snapshot.m_inventories.end(),
          convert_inventory_snapshot_inventories(account))));
      auto sequence =
        InventorySequence(account.m_id, stripped_snapshot.m_sequence);
      m_connection->execute(Viper::insert(
        get_inventory_sequences_row(), "inventory_sequences", &sequence));
      m_connection->execute(Viper::insert(get_inventory_excluded_orders_row(),
        "inventory_excluded_orders", boost::iterators::make_transform_iterator(
          stripped_snapshot.m_excluded_orders.begin(),
          convert_inventory_excluded_orders(account)),
        boost::iterators::make_transform_iterator(
          stripped_snapshot.m_excluded_orders.end(),
          convert_inventory_excluded_orders(account))));
    });
  }

  template<typename C>
  void SqlRiskDataStore<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_connection->close();
    m_open_state.close();
  }
}

#endif
