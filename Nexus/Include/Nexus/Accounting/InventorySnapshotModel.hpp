#ifndef NEXUS_INVENTORY_SNAPSHOT_MODEL_HPP
#define NEXUS_INVENTORY_SNAPSHOT_MODEL_HPP
#include <algorithm>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <Beam/Queries/Sequence.hpp>
#include "Nexus/Accounting/InventorySnapshot.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /** Maintains an account's InventorySnapshot by tracking its Orders. */
  class InventorySnapshotModel {
    public:

      /** Constructs an empty InventorySnapshotModel. */
      InventorySnapshotModel() = default;

      /**
       * Constructs an InventorySnapshotModel from an existing snapshot.
       * @param snapshot The snapshot used to seed this model.
       */
      explicit InventorySnapshotModel(const InventorySnapshot& snapshot);

      /**
       * Adds an Order to this model.
       * @param sequence The Order's submission sequence.
       * @param order The Order to track.
       */
      void add(Beam::Sequence sequence, std::shared_ptr<Order> order);

      /**
       * Updates this model with the contents of an ExecutionReport.
       * @param report The ExecutionReport to update this model with.
       */
      void update(const ExecutionReport& report);

      /** Returns the InventorySnapshot represented by this model. */
      InventorySnapshot make_snapshot() const;

    private:
      Portfolio<TrueAverageBookkeeper> m_portfolio;
      Beam::Sequence m_sequence;
      Beam::Sequence m_seed_sequence;
      std::unordered_map<OrderId, std::shared_ptr<Order>> m_live_orders;
      std::unordered_set<OrderId> m_pending_orders;

      void fold(const Order& order);
  };

  inline InventorySnapshotModel::InventorySnapshotModel(
    const InventorySnapshot& snapshot)
    : m_portfolio(
        Portfolio<TrueAverageBookkeeper>::Bookkeeper(snapshot.m_inventories)),
      m_sequence(snapshot.m_sequence),
      m_seed_sequence(snapshot.m_sequence),
      m_pending_orders(
        snapshot.m_excluded_orders.begin(), snapshot.m_excluded_orders.end()) {}

  inline void InventorySnapshotModel::add(
      Beam::Sequence sequence, std::shared_ptr<Order> order) {
    auto id = order->get_info().m_id;
    auto is_pending = m_pending_orders.erase(id) != 0;
    if(!is_pending && sequence <= m_seed_sequence) {
      return;
    }
    m_sequence = std::max(m_sequence, sequence);
    auto reports = order->get_publisher().get_snapshot();
    if(reports && !reports->empty() && is_terminal(reports->back().m_status)) {
      fold(*order);
    } else {
      m_live_orders.insert_or_assign(id, std::move(order));
    }
  }

  inline void InventorySnapshotModel::update(const ExecutionReport& report) {
    if(!is_terminal(report.m_status)) {
      return;
    }
    auto order = m_live_orders.find(report.m_id);
    if(order == m_live_orders.end()) {
      return;
    }
    fold(*order->second);
    m_live_orders.erase(order);
  }

  inline InventorySnapshot InventorySnapshotModel::make_snapshot() const {
    auto snapshot = InventorySnapshot();
    for(auto& inventory : m_portfolio.get_bookkeeper().get_inventory_range()) {
      snapshot.m_inventories.push_back(inventory);
    }
    snapshot.m_sequence = m_sequence;
    for(auto id : m_live_orders | std::views::keys) {
      snapshot.m_excluded_orders.push_back(id);
    }
    snapshot.m_excluded_orders.insert(snapshot.m_excluded_orders.end(),
      m_pending_orders.begin(), m_pending_orders.end());
    return snapshot;
  }

  inline void InventorySnapshotModel::fold(const Order& order) {
    if(auto reports = order.get_publisher().get_snapshot()) {
      for(auto& report : *reports) {
        m_portfolio.update(order.get_info().m_fields, report);
      }
    }
  }
}

#endif
