#ifndef NEXUS_INVENTORY_SNAPSHOT_HPP
#define NEXUS_INVENTORY_SNAPSHOT_HPP
#include <tuple>
#include <vector>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus {

  /** Stores a snapshot of inventories. */
  struct InventorySnapshot {

    /** The list of non-empty inventories. */
    std::vector<Inventory> m_inventories;

    /** The sequence that this snapshot is valid for (inclusive). */
    Beam::Sequence m_sequence;

    /** The list of Order ids excluded from this snapshot. */
    std::vector<OrderId> m_excluded_orders;

    bool operator ==(const InventorySnapshot&) const = default;
  };

  /**
   * Strips a snapshot of empty Inventory objects, used to avoid storing empty
   * inventories in a RiskDataStore.
   * @param snapshot The snapshot to trim.
   * @return A copy of the <i>snapshot</i> with all empty Inventory objects
   *         removed.
   */
  inline InventorySnapshot strip(InventorySnapshot snapshot) {
    std::erase_if(snapshot.m_inventories, [] (const auto& inventory) {
      return is_empty(inventory);
    });
    return snapshot;
  }

  /**
   * Returns a RiskPortfolio from an InventorySnapshot.
   * @param snapshot The InventorySnapshot used to build the portfolio.
   * @param account The account the portfolio represents.
   * @param client The OrderExecutionClient to query.
   * @return A triple consisting of the portfolio that was built, the Order
   *         query sequence that the portfolio is valid up to for the specified
   *         <i>account</i>, and the list of Orders excluded from the portfolio.
   */
  std::tuple<RiskPortfolio, Beam::Sequence,
      std::vector<std::shared_ptr<Order>>> make_portfolio(
        const InventorySnapshot& snapshot,
        const Beam::DirectoryEntry& account,
        IsOrderExecutionClient auto& client) {
    auto excluded_orders =
      load_orders(account, snapshot.m_excluded_orders, client);
    auto trailing_order_query = AccountQuery();
    trailing_order_query.set_index(account);
    trailing_order_query.set_range(
      Beam::increment(snapshot.m_sequence), Beam::Sequence::PRESENT);
    trailing_order_query.set_snapshot_limit(Beam::SnapshotLimit::UNLIMITED);
    auto trailing_orders_queue =
      std::make_shared<Beam::Queue<Nexus::SequencedOrder>>();
    client.query(trailing_order_query, trailing_orders_queue);
    auto sequence = snapshot.m_sequence;
    Beam::for_each(trailing_orders_queue, [&] (const auto& order) {
      excluded_orders.push_back(order.get_value());
      sequence = std::max(sequence, order.get_sequence());
    });
    auto portfolio =
      RiskPortfolio(RiskPortfolio::Bookkeeper(snapshot.m_inventories));
    return {std::move(portfolio), sequence, std::move(excluded_orders)};
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::InventorySnapshot> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::InventorySnapshot& value,
        unsigned int version) const {
      shuttle.shuttle("inventories", value.m_inventories);
      shuttle.shuttle("sequence", value.m_sequence);
      shuttle.shuttle("excluded_orders", value.m_excluded_orders);
    }
  };
}

#endif
