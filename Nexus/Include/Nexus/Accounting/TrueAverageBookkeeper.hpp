#ifndef NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#define NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"

namespace Nexus::Accounting {
namespace Details {
  template<typename T>
  struct ValueAccessor {
    using value_type = const typename T::second_type;
    using result_type = const value_type&;

    template<typename U>
    result_type operator ()(const U& value) const {
      return value.second;
    }
  };
}

  /**
   * Implements a Bookkeeper using true average bookkeeping.
   * @param <I> The type of Inventory to manage.
   */
  template<typename I>
  class TrueAverageBookkeeper {
    public:
      using Inventory = typename Bookkeeper<I>::Inventory;
      using Index = typename Bookkeeper<I>::Index;
      using Key = typename Bookkeeper<I>::Key;

      /** Constructs an empty bookkeeper. */
      TrueAverageBookkeeper() = default;

      /**
       * Constructs a bookkeeper with an initial inventory.
       * @param inventories The initial inventories.
       */
      explicit TrueAverageBookkeeper(
        const Beam::View<const Inventory>& inventories);

      void record(const Index& index, CurrencyId currency,
        Quantity quantity, Money cost_basis, Money fees);
      const Inventory& get_inventory(
        const Index& index, CurrencyId currency) const;
      const Inventory& get_total(CurrencyId currency) const;
      Beam::View<const Inventory> get_inventory_range() const;
      Beam::View<const Inventory> get_totals_range() const;
    private:
      std::unordered_map<Key, Inventory> m_inventories;
      std::unordered_map<CurrencyId, Inventory> m_totals;

      Inventory& internal_get_total(CurrencyId currency);
  };

  template<typename I>
  TrueAverageBookkeeper<I>::TrueAverageBookkeeper(
      const Beam::View<const Inventory>& inventories) {
    for(auto& inventory : inventories) {
      if(is_empty(inventory)) {
        continue;
      }
      m_inventories.insert(std::pair(inventory.m_position.m_key, inventory));
      auto& total = internal_get_total(inventory.m_position.m_key.m_currency);
      total.m_gross_profit_and_loss += inventory.m_gross_profit_and_loss;
      total.m_position.m_quantity += abs(inventory.m_position.m_quantity);
      total.m_position.m_cost_basis += abs(inventory.m_position.m_cost_basis);
      total.m_fees += inventory.m_fees;
      total.m_volume += inventory.m_volume;
      total.m_transaction_count += inventory.m_transaction_count;
    }
  }

  template<typename I>
  void TrueAverageBookkeeper<I>::record(const Index& index,
      CurrencyId currency, Quantity quantity, Money cost_basis, Money fees) {
    auto key = Key(index, currency);
    auto entry_iterator = m_inventories.find(key);
    if(entry_iterator == m_inventories.end()) {
      entry_iterator =
        m_inventories.insert(std::pair(key, Inventory(key))).first;
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

  template<typename I>
  const typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::get_inventory(
        const Index& index, CurrencyId currency) const {
    auto key = Key(index, currency);
    auto inventory_iterator = m_inventories.find(key);
    if(inventory_iterator == m_inventories.end()) {
      static auto empty_inventories =
        Beam::SynchronizedUnorderedMap<Key, Inventory>();
      return empty_inventories.GetOrInsert(key, [&] {
        return Inventory(key);
      });
    }
    return inventory_iterator->second;
  }

  template<typename I>
  const typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::get_total(CurrencyId currency) const {
    auto totals_iterator = m_totals.find(currency);
    if(totals_iterator == m_totals.end()) {
      static auto empty_totals =
        Beam::SynchronizedUnorderedMap<CurrencyId, Inventory>();
      return empty_totals.GetOrInsert(currency, [&] {
        auto totals = Inventory();
        totals.m_position.m_key.m_currency = currency;
        return totals;
      });
    }
    return totals_iterator->second;
  }

  template<typename I>
  Beam::View<const typename TrueAverageBookkeeper<I>::Inventory>
      TrueAverageBookkeeper<I>::get_inventory_range() const {
    using Accessor = Details::ValueAccessor<std::pair<const Key, Inventory>>;
    return Beam::View(
      boost::make_transform_iterator(m_inventories.begin(), Accessor()),
      boost::make_transform_iterator(m_inventories.end(), Accessor()));
  }

  template<typename I>
  Beam::View<const typename TrueAverageBookkeeper<I>::Inventory>
      TrueAverageBookkeeper<I>::get_totals_range() const {
    using Accessor =
      Details::ValueAccessor<std::pair<const CurrencyId, Inventory>>;
    return Beam::View(
      boost::make_transform_iterator(m_totals.begin(), Accessor()),
      boost::make_transform_iterator(m_totals.end(), Accessor()));
  }

  template<typename I>
  typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::internal_get_total(CurrencyId currency) {
    auto totals_iterator = m_totals.find(currency);
    if(totals_iterator == m_totals.end()) {
      totals_iterator = m_totals.insert(std::pair(currency, Inventory())).first;
      totals_iterator->second.m_position.m_key.m_currency = currency;
    }
    return totals_iterator->second;
  }
}

#endif
