#ifndef NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#define NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#include <ranges>
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"

namespace Nexus {

  /** Implements a Bookkeeper using true average bookkeeping. */
  class TrueAverageBookkeeper {
    public:

      /** Constructs an empty bookkeeper. */
      TrueAverageBookkeeper() = default;

      /**
       * Constructs a bookkeeper with an initial inventory.
       * @param inventories The initial inventories.
       */
      explicit TrueAverageBookkeeper(
        const Beam::View<const Inventory>& inventories);

      void record(const Ticker& ticker, Asset currency, Quantity quantity,
        Money cost_basis, Money fees);
      const Inventory& get_inventory(const Ticker& ticker) const;
      const Inventory& get_total(Asset asset) const;
      Beam::View<const Inventory> get_inventory_range() const;
      Beam::View<const Inventory> get_totals_range() const;

    private:
      std::unordered_map<Ticker, Inventory> m_inventories;
      std::unordered_map<Asset, Inventory> m_totals;

      Inventory& internal_get_total(Asset asset);
  };

  inline TrueAverageBookkeeper::TrueAverageBookkeeper(
      const Beam::View<const Inventory>& inventories) {
    for(auto& inventory : inventories) {
      if(is_empty(inventory)) {
        continue;
      }
      m_inventories.insert(std::pair(inventory.m_position.m_ticker, inventory));
      auto& total = internal_get_total(inventory.m_position.m_currency);
      total.m_gross_profit_and_loss += inventory.m_gross_profit_and_loss;
      total.m_position.m_quantity += abs(inventory.m_position.m_quantity);
      total.m_position.m_cost_basis += abs(inventory.m_position.m_cost_basis);
      total.m_fees += inventory.m_fees;
      total.m_volume += inventory.m_volume;
      total.m_transaction_count += inventory.m_transaction_count;
    }
  }

  inline void TrueAverageBookkeeper::record(const Ticker& ticker,
      Asset currency, Quantity quantity, Money cost_basis, Money fees) {
    auto entry_iterator = m_inventories.find(ticker);
    if(entry_iterator == m_inventories.end()) {
      entry_iterator = m_inventories.insert(
        std::pair(ticker, Inventory(ticker, currency))).first;
    }
    auto& entry = entry_iterator->second;
    entry.m_fees += fees;
    auto& total = internal_get_total(currency);
    total.m_fees += fees;
    if(quantity == 0) {
      return;
    }
    entry.m_volume += abs(quantity);
    ++entry.m_transaction_count;
    total.m_volume += abs(quantity);
    ++total.m_transaction_count;
    auto price = abs(cost_basis / quantity);
    auto remaining_quantity = abs(quantity);
    auto direction = [&] {
      if(quantity < 0) {
        return Quantity(-1);
      } else {
        return Quantity(1);
      }
    }();
    total.m_gross_profit_and_loss -= entry.m_gross_profit_and_loss;
    total.m_position.m_quantity -= abs(entry.m_position.m_quantity);
    total.m_position.m_cost_basis -= abs(entry.m_position.m_cost_basis);
    if((entry.m_position.m_quantity > 0 && quantity < 0) ||
        (entry.m_position.m_quantity < 0 && quantity > 0)) {
      auto transaction_reduction =
        std::min(remaining_quantity, abs(entry.m_position.m_quantity));
      auto average_price = get_average_price(entry.m_position);
      entry.m_gross_profit_and_loss +=
        -direction * transaction_reduction * (price - average_price);
      auto quantity_delta = direction * transaction_reduction;
      entry.m_position.m_quantity += quantity_delta;
      if(entry.m_position.m_quantity == 0) {
        entry.m_position.m_cost_basis = Money::ZERO;
      } else {
        entry.m_position.m_cost_basis += quantity_delta * average_price;
      }
      remaining_quantity -= transaction_reduction;
      if(remaining_quantity == 0) {
        total.m_gross_profit_and_loss += entry.m_gross_profit_and_loss;
        total.m_position.m_quantity += abs(entry.m_position.m_quantity);
        total.m_position.m_cost_basis += abs(entry.m_position.m_cost_basis);
        return;
      }
    }
    auto quantity_delta = direction * remaining_quantity;
    auto cost_basis_delta = direction * remaining_quantity * price;
    entry.m_position.m_quantity += quantity_delta;
    entry.m_position.m_cost_basis += cost_basis_delta;
    total.m_gross_profit_and_loss += entry.m_gross_profit_and_loss;
    total.m_position.m_quantity += abs(entry.m_position.m_quantity);
    total.m_position.m_cost_basis += abs(entry.m_position.m_cost_basis);
  }

  inline const Inventory& TrueAverageBookkeeper::get_inventory(
      const Ticker& ticker) const {
    auto inventory_iterator = m_inventories.find(ticker);
    if(inventory_iterator == m_inventories.end()) {
      static auto empty_inventories =
        Beam::SynchronizedUnorderedMap<Ticker, Inventory>();
      return empty_inventories.get_or_insert(ticker, [&] {
        return Inventory(ticker, Asset());
      });
    }
    return inventory_iterator->second;
  }

  inline const Inventory& TrueAverageBookkeeper::get_total(Asset asset) const {
    auto totals_iterator = m_totals.find(asset);
    if(totals_iterator == m_totals.end()) {
      static auto empty_totals =
        Beam::SynchronizedUnorderedMap<Asset, Inventory>();
      return empty_totals.get_or_insert(asset, [&] {
        return Inventory(Ticker(), asset);
      });
    }
    return totals_iterator->second;
  }

  inline Beam::View<const Inventory>
      TrueAverageBookkeeper::get_inventory_range() const {
    auto values = m_inventories | std::views::values;
    return Beam::View(values.begin(), values.end());
  }

  inline Beam::View<const Inventory>
      TrueAverageBookkeeper::get_totals_range() const {
    auto values = m_totals | std::views::values;
    return Beam::View(values.begin(), values.end());
  }

  inline Inventory& TrueAverageBookkeeper::internal_get_total(Asset asset) {
    auto totals_iterator = m_totals.find(asset);
    if(totals_iterator == m_totals.end()) {
      totals_iterator =
        m_totals.insert(std::pair(asset, Inventory(Ticker(), asset))).first;
    }
    return totals_iterator->second;
  }
}

#endif
