#ifndef NEXUS_RISK_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_RISK_SERVICE_SQL_DEFINITIONS_HPP
#include <cstdint>
#include <Beam/Sql/Conversions.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus {

  /** Stores an inventory and the account it belongs to. */
  struct InventoryEntry {

    /** The account the inventory belongs to. */
    std::uint32_t m_account;

    /** The inventory stored. */
    Inventory m_inventory;
  };

  /** Returns a row representing a InventoryEntry. */
  inline const auto& get_inventory_entries_row() {
    static auto ROW = Viper::Row<InventoryEntry>().
      add_column("account", &InventoryEntry::m_account).
        extend(Viper::Row<Inventory>().
          extend(Viper::Row<Position>().
            extend(Viper::Row<Ticker>().
              add_column("symbol", Viper::varchar(16),
                [] (auto& row) {
                  return row.get_symbol();
                },
                [] (auto& row, auto column) {
                  row = Ticker(std::move(column), row.get_venue());
                }).
              add_column("venue", Viper::varchar(16),
                [] (auto& row) {
                  return row.get_venue();
                },
                [] (auto& row, auto column) {
                  row = Ticker(row.get_symbol(), column);
                }),
              &Position::m_ticker).
            add_column("currency", &Position::m_currency).
            add_column("quantity", &Position::m_quantity).
            add_column("cost_basis", &Position::m_cost_basis),
            &Inventory::m_position).
          add_column(
            "gross_profit_and_loss", &Inventory::m_gross_profit_and_loss).
          add_column("fees", &Inventory::m_fees).
          add_column("volume", &Inventory::m_volume).
          add_column("transaction_count", &Inventory::m_transaction_count),
          &InventoryEntry::m_inventory).
        add_index("account", "account");
    return ROW;
  }

  /** Converts a RiskInventory into a InventoryEntry. */
  inline auto convert_inventory_snapshot_inventories(
      const Beam::DirectoryEntry& account) {
    return [=] (const Inventory& inventory) {
      return InventoryEntry(account.m_id, inventory);
    };
  }

  /** Stores a sequence and the account it belongs to. */
  struct InventorySequence {

    /** The account the sequence belongs to. */
    std::uint32_t m_account;

    /** The sequence stored. */
    Beam::Sequence m_sequence;
  };

  /** Returns a row representing a SequenceEntry. */
  inline const auto& get_inventory_sequences_row() {
    static auto ROW = Viper::Row<InventorySequence>().
      add_column("account", &InventorySequence::m_account).
      add_column("sequence", &InventorySequence::m_sequence).
      add_index("account", "account");
    return ROW;
  }

  /** Stores an excluded order id and the account it belongs to. */
  struct InventoryExcludedOrderId {

    /** The account the order belongs to. */
    std::uint32_t m_account;

    /** The excluded order id. */
    OrderId m_id;
  };

  /** Returns a row representing a InventoryExcludedOrderId. */
  inline const auto& get_inventory_excluded_orders_row() {
    static auto ROW = Viper::Row<InventoryExcludedOrderId>().
      add_column("account", &InventoryExcludedOrderId::m_account).
      add_column("id", &InventoryExcludedOrderId::m_id).
      add_index("account", "account");
    return ROW;
  }

  /** Converts an order id into a InventoryExcludedOrderId. */
  inline auto convert_inventory_excluded_orders(
      const Beam::DirectoryEntry& account) {
    return [=] (OrderId id) {
      return InventoryExcludedOrderId(account.m_id, id);
    };
  }
}

#endif
