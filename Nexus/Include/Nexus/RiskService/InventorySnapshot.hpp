#ifndef NEXUS_INVENTORY_SNAPSHOT_HPP
#define NEXUS_INVENTORY_SNAPSHOT_HPP
#include <tuple>
#include <vector>
#include <Beam/Queries/Sequence.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

namespace Nexus::RiskService {

  /** Stores a snapshot of inventories. */
  struct InventorySnapshot {

    /** The list of non-empty inventories. */
    std::vector<RiskInventory> m_inventories;

    /** The sequence that this snapshot is valid for (inclusive). */
    Beam::Queries::Sequence m_sequence;

    /** The list of Order ids excluded from this snapshot. */
    std::vector<OrderExecutionService::OrderId> m_excludedOrders;

    bool operator ==(const InventorySnapshot& snapshot) const = default;
  };

  /**
   * Returns a RiskPortfolio from an InventorySnapshot.
   * @param snapshot The InventorySnapshot used to build the portfolio.
   * @param account The account the portfolio represents.
   * @param markets The MarketDatabase to query.
   * @param client The OrderExecutionClient to query.
   * @return A triple consisting of the portfolio that was built, the Order
   *         query sequence that the portfolio is valid up to for the specified
   *         <i>account</i>, and the list of Orders excluded from the portfolio.
   */
  template<typename OrderExecutionClient>
  std::tuple<RiskPortfolio, Beam::Queries::Sequence,
      std::vector<const OrderExecutionService::Order*>> MakePortfolio(
      const InventorySnapshot& snapshot,
      const Beam::ServiceLocator::DirectoryEntry& account,
      MarketDatabase markets, OrderExecutionClient& client) {
    auto excludedOrders = OrderExecutionService::LoadOrderIds(account,
      snapshot.m_excludedOrders, client);
    auto trailingOrderQuery = OrderExecutionService::AccountQuery();
    trailingOrderQuery.SetIndex(account);
    trailingOrderQuery.SetRange(Beam::Queries::Increment(snapshot.m_sequence),
      Beam::Queries::Sequence::Present());
    trailingOrderQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    auto trailingOrdersQueue = std::make_shared<
      Beam::Queue<Nexus::OrderExecutionService::SequencedOrder>>();
    client.QueryOrderSubmissions(trailingOrderQuery, trailingOrdersQueue);
    auto sequence = snapshot.m_sequence;
    Beam::ForEach(trailingOrdersQueue, [&] (const auto& order) {
      excludedOrders.push_back(order.GetValue());
      sequence = std::max(sequence, order.GetSequence());
    });
    auto portfolio = RiskPortfolio(std::move(markets),
      RiskPortfolio::Bookkeeper(snapshot.m_inventories));
    return {std::move(portfolio), sequence, std::move(excludedOrders)};
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::RiskService::InventorySnapshot> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::RiskService::InventorySnapshot& value, unsigned int version) {
      shuttle.Shuttle("inventories", value.m_inventories);
      shuttle.Shuttle("sequence", value.m_sequence);
      shuttle.Shuttle("excluded_orders", value.m_excludedOrders);
    }
  };
}

#endif
