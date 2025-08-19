#ifndef NEXUS_INVENTORY_SNAPSHOT_HPP
#define NEXUS_INVENTORY_SNAPSHOT_HPP
#include <tuple>
#include <vector>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Stores a snapshot of inventories. */
  struct InventorySnapshot {

    /** The list of non-empty inventories. */
    std::vector<Accounting::Inventory> m_inventories;

    /** The sequence that this snapshot is valid for (inclusive). */
    Beam::Queries::Sequence m_sequence;

    /** The list of Order ids excluded from this snapshot. */
    std::vector<OrderExecutionService::OrderId> m_excluded_orders;

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
    snapshot.m_inventories.erase(std::remove_if(snapshot.m_inventories.begin(),
      snapshot.m_inventories.end(), [] (const auto& inventory) {
        return is_empty(inventory);
      }), snapshot.m_inventories.end());
    return snapshot;
  }

  /**
   * Returns a RiskPortfolio from an InventorySnapshot.
   * @param snapshot The InventorySnapshot used to build the portfolio.
   * @param account The account the portfolio represents.
   * @param venues The available venues.
   * @param client The OrderExecutionClient to query.
   * @return A triple consisting of the portfolio that was built, the Order
   *         query sequence that the portfolio is valid up to for the specified
   *         <i>account</i>, and the list of Orders excluded from the portfolio.
   */
  std::tuple<RiskPortfolio, Beam::Queries::Sequence,
      std::vector<std::shared_ptr<const OrderExecutionService::Order>>>
        make_portfolio(const InventorySnapshot& snapshot,
          const Beam::ServiceLocator::DirectoryEntry& account,
          VenueDatabase venues,
          OrderExecutionService::IsOrderExecutionClient auto& client) {
    auto excluded_orders = OrderExecutionService::load_orders(
      account, snapshot.m_excluded_orders, client);
    auto trailing_order_query = OrderExecutionService::AccountQuery();
    trailing_order_query.SetIndex(account);
    trailing_order_query.SetRange(Beam::Queries::Increment(snapshot.m_sequence),
      Beam::Queries::Sequence::Present());
    trailing_order_query.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    auto trailing_orders_queue = std::make_shared<
      Beam::Queue<Nexus::OrderExecutionService::SequencedOrder>>();
    client.query(trailing_order_query, trailing_orders_queue);
    auto sequence = snapshot.m_sequence;
    Beam::ForEach(trailing_orders_queue, [&] (const auto& order) {
      excluded_orders.push_back(order.GetValue());
      sequence = std::max(sequence, order.GetSequence());
    });
    auto portfolio = RiskPortfolio(
      RiskPortfolio::Bookkeeper(snapshot.m_inventories), std::move(venues));
    return {std::move(portfolio), sequence, std::move(excluded_orders)};
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::InventorySnapshot> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::RiskService::InventorySnapshot& value,
        unsigned int version) const {
      shuttle.Shuttle("inventories", value.m_inventories);
      shuttle.Shuttle("sequence", value.m_sequence);
      shuttle.Shuttle("excluded_orders", value.m_excluded_orders);
    }
  };
}

#endif
