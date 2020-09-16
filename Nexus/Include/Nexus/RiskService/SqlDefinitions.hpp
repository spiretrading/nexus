#ifndef NEXUS_RISK_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_RISK_SERVICE_SQL_DEFINITIONS_HPP
#include <cstdint>
#include <Beam/Sql/Conversions.hpp>
#include <Viper/Row.hpp>
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Stores an inventory and the account it belongs to. */
  struct InventoryEntry {

    /** The account the inventory belongs to. */
    std::uint32_t m_account;

    /** The inventory stored. */
    RiskInventory m_inventory;
  };

  /** Returns a row representing a InventoryEntry. */
  inline const auto& GetInventoryEntriesRow() {
    static auto ROW = Viper::Row<InventoryEntry>().
      add_column("account", &InventoryEntry::m_account).
      extend(Viper::Row<RiskInventory>().
        extend(Viper::Row<RiskPosition>().
          extend(Viper::Row<RiskPosition::Key>().
            extend(Viper::Row<Security>().
              add_column("symbol", Viper::varchar(16),
                [] (auto& row) {
                  return row.GetSymbol();
                },
                [] (auto& row, auto column) {
                  row = Security(std::move(column), row.GetMarket(),
                    row.GetCountry());
                }).
              add_column("market", Viper::varchar(16),
                [] (auto& row) {
                  return row.GetMarket();
                },
                [] (auto& row, auto column) {
                  row = Security(row.GetSymbol(), column, row.GetCountry());
                }).
              add_column("country",
                [] (auto& row) {
                  return row.GetCountry();
                },
                [] (auto& row, auto column) {
                  row = Security(row.GetSymbol(), row.GetMarket(), column);
                }),
              [] (auto& entry) -> auto& {
                return entry.m_index;
              }).
            add_column("currency",
              [] (auto& entry) -> auto& {
                return entry.m_currency;
              }),
            [] (auto& entry) -> auto& {
              return entry.m_key;
            }).
          add_column("quantity",
            [] (auto& entry) -> auto& {
              return entry.m_quantity;
            }).
          add_column("cost_basis",
            [] (auto& entry) -> auto& {
              return entry.m_costBasis;
            }),
          [] (auto& entry) -> auto& {
            return entry.m_position;
          }).
        add_column("gross_profit_and_loss",
          [] (auto& entry) -> auto& {
            return entry.m_grossProfitAndLoss;
          }).
        add_column("fees",
          [] (auto& entry) -> auto& {
            return entry.m_fees;
          }).
        add_column("volume",
          [] (auto& entry) -> auto& {
            return entry.m_volume;
          }).
        add_column("transaction_count",
          [] (auto& entry) -> auto& {
            return entry.m_transactionCount;
          }), &InventoryEntry::m_inventory).
      add_index("account", "account");
    return ROW;
  }

  /** Converts a RiskInventory into a InventoryEntry. */
  inline auto ConvertInventorySnapshotInventories(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return [=] (const RiskInventory& inventory) {
      return InventoryEntry{account.m_id, inventory};
    };
  }

  /** Stores a sequence and the account it belongs to. */
  struct InventorySequence {

    /** The account the sequence belongs to. */
    std::uint32_t m_account;

    /** The sequence stored. */
    Beam::Queries::Sequence m_sequence;
  };

  /** Returns a row representing a SequenceEntry. */
  inline const auto& GetInventorySequencesRow() {
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
    OrderExecutionService::OrderId m_id;
  };

  /** Returns a row representing a InventoryExcludedOrderId. */
  inline const auto& GetInventoryExcludedOrdersRow() {
    static auto ROW = Viper::Row<InventoryExcludedOrderId>().
      add_column("account", &InventoryExcludedOrderId::m_account).
      add_column("id", &InventoryExcludedOrderId::m_id).
      add_index("account", "account");
    return ROW;
  }

  /** Converts an order id into a InventoryExcludedOrderId. */
  inline auto ConvertInventoryExcludedOrders(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return [=] (OrderExecutionService::OrderId id) {
      return InventoryExcludedOrderId{account.m_id, id};
    };
  }
}

#endif
