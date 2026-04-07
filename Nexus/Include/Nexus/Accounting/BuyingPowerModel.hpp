#ifndef NEXUS_BUYING_POWER_MODEL_HPP
#define NEXUS_BUYING_POWER_MODEL_HPP
#include <algorithm>
#include <unordered_map>
#include <vector>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Tracks the amount of buying power used up by a series of Orders. */
  class BuyingPowerModel {
    public:

      /**
       * Returns <code>true</code> iff an Order has been accounted for.
       * @param id The Order id to check.
       * @return <code>true</code> iff the Order with the specified <i>id</i>
       *         has previously been accounted for.
       */
      bool has_order(OrderId id) const;

      /**
       * Returns the buying power used in a particular Currency.
       * @param currency The Currency to lookup.
       * @return The buying power used in the <i>currency</i>.
       */
      Money get_buying_power(CurrencyId currency) const;

      /**
       * Tracks a submission and returns the updated buying power.
       * @param id The id used to track this submission.
       * @param fields The OrderFields storing the details of the
       *        submission.
       * @param expected_price The expected price of the Order, this may differ
       *        from the price that the Order is submitted for.
       * @return The updated buying power for the submission's Currency.
       */
      Money submit(OrderId id, const OrderFields& fields, Money expected_price);

      /**
       * Updates this model with the contents of an ExecutionReport.
       * @param report The ExecutionReport to update this model with.
       */
      void update(const ExecutionReport& report);

    private:
      struct OrderEntry {
        OrderId m_id;
        OrderFields m_fields;
        Money m_expected_price;
        Quantity m_remaining_quantity;

        OrderEntry(OrderId id, const OrderFields& fields, Money expected_price);
      };
      struct BuyingPowerEntry {
        std::vector<OrderEntry> m_asks;
        std::vector<OrderEntry> m_bids;
        Money m_expenditure;
        Quantity m_quantity;
      };
      std::unordered_map<OrderId, OrderFields> m_order_fields;
      std::unordered_map<Security, BuyingPowerEntry> m_buying_power_entries;
      std::unordered_map<CurrencyId, Money> m_buying_power;

      static Money compute_buying_power(
        const std::vector<OrderEntry>& entries, Quantity offset);
      static Money compute_buying_power(const BuyingPowerEntry& entry);
  };

  inline BuyingPowerModel::OrderEntry::OrderEntry(
    OrderId id, const OrderFields& fields, Money expected_price)
    : m_id(id),
      m_fields(fields),
      m_expected_price(expected_price),
      m_remaining_quantity(fields.m_quantity) {}

  inline bool BuyingPowerModel::has_order(OrderId id) const {
    return m_order_fields.contains(id);
  }

  inline Money BuyingPowerModel::get_buying_power(CurrencyId currency) const {
    auto currency_iterator = m_buying_power.find(currency);
    if(currency_iterator == m_buying_power.end()) {
      return Money::ZERO;
    }
    return currency_iterator->second;
  }

  inline Money BuyingPowerModel::submit(
      OrderId id, const OrderFields& fields, Money expected_price) {
    auto& entry = m_buying_power_entries[fields.m_security];
    auto& buying_power = m_buying_power[fields.m_currency];
    buying_power -= compute_buying_power(entry);
    auto& order_entries = pick(fields.m_side, entry.m_asks, entry.m_bids);
    auto order_entry = OrderEntry(id, fields, expected_price);
    auto insert_iterator = std::lower_bound(order_entries.begin(),
      order_entries.end(), order_entry, [] (const auto& lhs, const auto& rhs) {
        return (lhs.m_fields.m_side == Side::ASK &&
          lhs.m_expected_price < rhs.m_expected_price) ||
          (lhs.m_fields.m_side == Side::BID &&
            lhs.m_expected_price > rhs.m_expected_price);
      });
    if(insert_iterator != order_entries.end() &&
        insert_iterator->m_remaining_quantity == 0) {
      *insert_iterator = order_entry;
    } else {
      order_entries.insert(insert_iterator, order_entry);
    }
    buying_power += compute_buying_power(entry);
    m_order_fields.insert(std::pair(id, fields));
    return buying_power;
  }

  inline void BuyingPowerModel::update(const ExecutionReport& report) {
    if(report.m_status == OrderStatus::PENDING_NEW ||
        report.m_status == OrderStatus::SUSPENDED ||
        report.m_status == OrderStatus::PENDING_CANCEL ||
        report.m_status == OrderStatus::NEW ||
        report.m_status == OrderStatus::CANCEL_REJECT) {
      return;
    }
    auto& order_fields = m_order_fields.at(report.m_id);
    auto& buying_power_entry =
      m_buying_power_entries.at(order_fields.m_security);
    auto& buying_power = m_buying_power[order_fields.m_currency];
    buying_power -= compute_buying_power(buying_power_entry);
    auto& order_entries = pick(order_fields.m_side,
      buying_power_entry.m_asks, buying_power_entry.m_bids);
    for(auto& order_entry : order_entries) {
      if(report.m_id == order_entry.m_id) {
        if(is_terminal(report.m_status)) {
          order_entry.m_remaining_quantity = 0;
        } else {
          order_entry.m_remaining_quantity -= report.m_last_quantity;
        }
        break;
      }
    }
    auto last_quantity = report.m_last_quantity;
    if((order_fields.m_side == Side::BID &&
        buying_power_entry.m_quantity < 0) ||
        (order_fields.m_side == Side::ASK &&
          buying_power_entry.m_quantity > 0)) {
      auto delta =
        std::min(abs(buying_power_entry.m_quantity), last_quantity);
      buying_power_entry.m_expenditure -=
        get_direction(get_opposite(order_fields.m_side)) * delta *
          (buying_power_entry.m_expenditure / buying_power_entry.m_quantity);
      buying_power_entry.m_quantity +=
        get_direction(order_fields.m_side) * delta;
      last_quantity -= delta;
    }
    buying_power_entry.m_quantity +=
      get_direction(order_fields.m_side) * last_quantity;
    buying_power_entry.m_expenditure +=
      get_direction(order_fields.m_side) * last_quantity * report.m_last_price;
    buying_power += compute_buying_power(buying_power_entry);
  }

  inline Money BuyingPowerModel::compute_buying_power(
      const std::vector<OrderEntry>& entries, Quantity offset) {
    auto buying_power = Money();
    for(auto& order_entry : entries) {
      if(offset == 0) {
        buying_power +=
          order_entry.m_remaining_quantity * order_entry.m_expected_price;
      } else if(order_entry.m_remaining_quantity < offset) {
        offset -= order_entry.m_remaining_quantity;
      } else {
        buying_power += (order_entry.m_remaining_quantity - offset) *
          order_entry.m_expected_price;
        offset = 0;
      }
    }
    return buying_power;
  }

  inline Money BuyingPowerModel::compute_buying_power(
      const BuyingPowerEntry& entry) {
    auto ask_buying_power = Money();
    auto bid_buying_power = Money();
    if(entry.m_quantity >= 0) {
      ask_buying_power = compute_buying_power(entry.m_asks, entry.m_quantity);
      bid_buying_power =
        compute_buying_power(entry.m_bids, 0) + entry.m_expenditure;
    } else {
      ask_buying_power =
        compute_buying_power(entry.m_asks, 0) - entry.m_expenditure;
      bid_buying_power = compute_buying_power(entry.m_bids, -entry.m_quantity);
    }
    return std::max(ask_buying_power, bid_buying_power);
  }
}

#endif
