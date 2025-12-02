#ifndef NEXUS_POSITION_ORDER_BOOK_HPP
#define NEXUS_POSITION_ORDER_BOOK_HPP
#include <functional>
#include <memory>
#include <ostream>
#include <ranges>
#include <unordered_map>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <Beam/Utilities/CachedValue.hpp>
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /**
   * Maintains an Order book for the purpose of keeping track of positions and
   * opening/closing orders.
   */
  class PositionOrderBook {
    public:

      /** Stores a single position. */
      struct PositionEntry {

        /** The position's Security. */
        Security m_security;

        /** The position's quantity. */
        Quantity m_quantity;

        bool operator ==(const PositionEntry&) const = default;
      };

      /** Constructs a PositionOrderBook. */
      PositionOrderBook() noexcept;

      /**
       * Constructs a PositionOrderBook.
       * @param positions The initial set of positions to populate the book
       *        with.
       */
      explicit PositionOrderBook(Beam::View<const Inventory> positions);

      /** Returns all live Orders. */
      const std::vector<std::shared_ptr<Order>>& get_live_orders() const;

      /**
       * Returns all opening Orders.
       * @return The list of all all opening Orders.
       */
      const std::vector<std::shared_ptr<Order>>& get_opening_orders() const;

      /**
       * Returns all Positions.
       * @return The list of all all Positions.
       */
      const std::vector<PositionEntry>& get_positions() const;

      /**
       * Returns <code>true</code> iff an Order submission would result in an
       * opening Order.
       * @param fields The OrderFields being submitted.
       * @return <code>true</code> iff the <i>fields</i> would result in an
       *         opening Order, otherwise the <i>fields</i> represent a closing
       *         Order.
       */
      bool test_opening_order_submission(const OrderFields& fields) const;

      /**
       * Adds an Order to this book.
       * @param order The Order to add.
       */
      void add(std::shared_ptr<Order> order);

      /**
       * Updates an Order with an ExecutionReport.
       * @param report The ExecutionReport with the details of the update.
       */
      void update(const ExecutionReport& report);

    private:
      struct OrderEntry {
        std::shared_ptr<Order> m_order;
        Quantity m_remaining_quantity;
        int m_sequence_number;

        OrderEntry(std::shared_ptr<Order> order, int sequence_number) noexcept;
      };
      struct SecurityEntry {
        std::vector<OrderEntry> m_asks;
        std::vector<OrderEntry> m_bids;
        Quantity m_position;
        Quantity m_ask_open_quantity;
        Quantity m_bid_open_quantity;
      };
      std::unordered_map<Security, SecurityEntry> m_security_entries;
      std::unordered_map<OrderId, OrderFields> m_fields;
      int m_order_sequence_number;
      Beam::CachedValue<std::vector<std::shared_ptr<Order>>> m_live_orders;
      Beam::CachedValue<std::vector<std::shared_ptr<Order>>> m_opening_orders;
      Beam::CachedValue<std::vector<PositionEntry>> m_positions;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const PositionOrderBook::PositionEntry& entry) {
    return out << '(' << entry.m_security << ' ' << entry.m_quantity << ')';
  }

  inline PositionOrderBook::OrderEntry::OrderEntry(
    std::shared_ptr<Order> order, int sequence_number) noexcept
    : m_order(std::move(order)),
      m_remaining_quantity(m_order->get_info().m_fields.m_quantity),
      m_sequence_number(sequence_number) {}

  inline PositionOrderBook::PositionOrderBook() noexcept
    : m_order_sequence_number(0) {
    m_live_orders.set_computation([this] {
      auto orders = std::vector<std::shared_ptr<Order>>();
      for(auto& entry : m_security_entries | std::views::values) {
        for(auto& order_entry : entry.m_asks) {
          orders.push_back(order_entry.m_order);
        }
        for(auto& order_entry : entry.m_bids) {
          orders.push_back(order_entry.m_order);
        }
      }
      return orders;
    });
    m_opening_orders.set_computation([this] {
      auto orders = std::vector<std::shared_ptr<Order>>();
      for(auto& entry : m_security_entries | std::views::values) {
        if(entry.m_position == 0) {
          for(auto& order_entry : entry.m_asks) {
            orders.push_back(order_entry.m_order);
          }
          for(auto& order_entry : entry.m_bids) {
            orders.push_back(order_entry.m_order);
          }
        } else if(entry.m_position > 0) {
          auto remaining_quantity = entry.m_position;
          for(auto& order_entry : entry.m_asks) {
            if(remaining_quantity <= 0) {
              orders.push_back(order_entry.m_order);
            } else if(remaining_quantity >= order_entry.m_remaining_quantity) {
              remaining_quantity -= order_entry.m_remaining_quantity;
            } else {
              remaining_quantity -= order_entry.m_remaining_quantity;
              orders.push_back(order_entry.m_order);
            }
          }
          for(auto& order_entry : entry.m_bids) {
            orders.push_back(order_entry.m_order);
          }
        } else {
          for(auto& order_entry : entry.m_asks) {
            orders.push_back(order_entry.m_order);
          }
          auto remaining_quantity = -entry.m_position;
          for(auto& order_entry : entry.m_bids) {
            if(remaining_quantity <= 0) {
              orders.push_back(order_entry.m_order);
            } else if(remaining_quantity >= order_entry.m_remaining_quantity) {
              remaining_quantity -= order_entry.m_remaining_quantity;
            } else {
              remaining_quantity -= order_entry.m_remaining_quantity;
              orders.push_back(order_entry.m_order);
            }
          }
        }
      }
      return orders;
    });
    m_positions.set_computation([this] {
      auto positions = std::vector<PositionEntry>();
      for(auto& security_entry_pair : m_security_entries) {
        if(security_entry_pair.second.m_position != 0) {
          positions.push_back(PositionEntry(security_entry_pair.first,
            security_entry_pair.second.m_position));
        }
      }
      return positions;
    });
  }

  inline PositionOrderBook::PositionOrderBook(
      Beam::View<const Inventory> positions)
      : PositionOrderBook() {
    for(auto& position : positions) {
      auto& security_entry = m_security_entries[position.m_position.m_security];
      security_entry.m_position = position.m_position.m_quantity;
    }
  }

  inline const std::vector<std::shared_ptr<Order>>&
      PositionOrderBook::get_live_orders() const {
    return *m_live_orders;
  }

  inline const std::vector<std::shared_ptr<Order>>&
      PositionOrderBook::get_opening_orders() const {
    return *m_opening_orders;
  }

  inline const std::vector<PositionOrderBook::PositionEntry>&
      PositionOrderBook::get_positions() const {
    return *m_positions;
  }

  inline bool PositionOrderBook::test_opening_order_submission(
      const OrderFields& fields) const {
    auto security_entry_iterator = m_security_entries.find(fields.m_security);
    if(security_entry_iterator == m_security_entries.end()) {
      return true;
    }
    const auto& security_entry = security_entry_iterator->second;
    if(security_entry.m_position == 0) {
      return true;
    }
    if(Nexus::get_side(security_entry.m_position) == fields.m_side) {
      return true;
    }
    auto open_quantity = pick(fields.m_side, security_entry.m_ask_open_quantity,
      security_entry.m_bid_open_quantity);
    if(open_quantity + fields.m_quantity > abs(security_entry.m_position)) {
      return true;
    }
    return false;
  }

  inline void PositionOrderBook::add(std::shared_ptr<Order> order) {
    auto& fields = order->get_info().m_fields;
    m_fields.emplace(order->get_info().m_id, fields);
    auto& security_entry = m_security_entries[fields.m_security];
    auto& orders = pick(fields.m_side, security_entry.m_asks,
      security_entry.m_bids);
    auto& open_quantity = pick(fields.m_side,
      security_entry.m_ask_open_quantity, security_entry.m_bid_open_quantity);
    open_quantity += fields.m_quantity;
    auto entry = OrderEntry(order, m_order_sequence_number);
    ++m_order_sequence_number;
    auto insert_iterator = std::lower_bound(orders.begin(), orders.end(), entry,
      [] (const auto& lhs, const auto& rhs) {
        return std::tie(
          lhs.m_order->get_info().m_fields, lhs.m_sequence_number) <
            std::tie(rhs.m_order->get_info().m_fields, rhs.m_sequence_number);
      });
    orders.insert(insert_iterator, entry);
    m_live_orders.invalidate();
    m_opening_orders.invalidate();
  }

  inline void PositionOrderBook::update(const ExecutionReport& report) {
    if(report.m_last_quantity == 0 && !is_terminal(report.m_status)) {
      return;
    }
    auto fields_iterator = m_fields.find(report.m_id);
    if(fields_iterator == m_fields.end()) {
      return;
    }
    auto& fields = fields_iterator->second;
    auto security_entry_iterator = m_security_entries.find(fields.m_security);
    if(security_entry_iterator == m_security_entries.end()) {
      return;
    }
    auto& security_entry = security_entry_iterator->second;
    auto& orders =
      pick(fields.m_side, security_entry.m_asks, security_entry.m_bids);
    auto entry_iterator = std::find_if(orders.begin(), orders.end(),
      [&] (const auto& entry) {
        return entry.m_order->get_info().m_id == report.m_id;
      });
    if(entry_iterator == orders.end()) {
      return;
    }
    if(is_terminal(report.m_status)) {
      m_live_orders.invalidate();
    }
    m_opening_orders.invalidate();
    if(report.m_last_quantity != 0) {
      m_positions.invalidate();
    }
    security_entry.m_position +=
      get_direction(fields.m_side) * report.m_last_quantity;
    auto& open_quantity = pick(fields.m_side,
      security_entry.m_ask_open_quantity, security_entry.m_bid_open_quantity);
    open_quantity -= report.m_last_quantity;
    auto& entry = *entry_iterator;
    entry.m_remaining_quantity -= report.m_last_quantity;
    if(entry.m_remaining_quantity == 0 || is_terminal(report.m_status)) {
      open_quantity -= entry.m_remaining_quantity;
      m_fields.erase(report.m_id);
      orders.erase(entry_iterator);
    }
  }
}

#endif
