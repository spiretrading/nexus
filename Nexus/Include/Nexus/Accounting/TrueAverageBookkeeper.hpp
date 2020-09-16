#ifndef NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#define NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#include <unordered_map>
#include <Beam/Collections/View.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"

namespace Nexus::Accounting {

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

      void RecordTransaction(const Index& index, CurrencyId currency,
        Quantity quantity, Money costBasis, Money fees);

      const Inventory& GetInventory(const Index& index,
        CurrencyId currency) const;

      const Inventory& GetTotal(CurrencyId currency) const;

      Beam::View<std::pair<const Key, Inventory>> GetInventoryRange() const;

      Beam::View<std::pair<const CurrencyId, Inventory>> GetTotalsRange() const;

    private:
      mutable std::unordered_map<Key, Inventory> m_inventories;
      mutable std::unordered_map<CurrencyId, Inventory> m_totals;

      Inventory& InternalGetTotal(CurrencyId currency);
  };

  template<typename I>
  TrueAverageBookkeeper<I>::TrueAverageBookkeeper(
      const Beam::View<const Inventory>& inventories) {
    for(auto& inventory : inventories) {
      if(IsEmpty(inventory)) {
        continue;
      }
      m_inventories.insert(std::pair(inventory.m_position.m_key, inventory));
      auto& total = InternalGetTotal(inventory.m_position.m_key.m_currency);
      total.m_grossProfitAndLoss += inventory.m_grossProfitAndLoss;
      total.m_position.m_quantity += Abs(inventory.m_position.m_quantity);
      total.m_position.m_costBasis += Abs(inventory.m_position.m_costBasis);
      total.m_fees += inventory.m_fees;
      total.m_volume += inventory.m_volume;
      total.m_transactionCount += inventory.m_transactionCount;
    }
  }

  template<typename I>
  void TrueAverageBookkeeper<I>::RecordTransaction(const Index& index,
      CurrencyId currency, Quantity quantity, Money costBasis, Money fees) {
    auto key = Key(index, currency);
    auto entryIterator = m_inventories.find(key);
    if(entryIterator == m_inventories.end()) {
      entryIterator = m_inventories.insert(
        std::pair(key, Inventory(key))).first;
    }
    auto& entry = entryIterator->second;
    auto& total = InternalGetTotal(currency);
    entry.m_fees += fees;
    entry.m_volume += Abs(quantity);
    ++entry.m_transactionCount;
    total.m_fees += fees;
    total.m_volume += Abs(quantity);
    ++total.m_transactionCount;
    if(quantity == 0) {
      return;
    }
    auto price = Abs(costBasis / quantity);
    auto remainingQuantity = Abs(quantity);
    auto direction = [&] {
      if(quantity < 0) {
        return Quantity(-1);
      } else {
        return Quantity(1);
      }
    }();
    total.m_grossProfitAndLoss -= entry.m_grossProfitAndLoss;
    total.m_position.m_quantity -= Abs(entry.m_position.m_quantity);
    total.m_position.m_costBasis -= Abs(entry.m_position.m_costBasis);
    if((entry.m_position.m_quantity > 0 && quantity < 0) ||
        (entry.m_position.m_quantity < 0 && quantity > 0)) {
      auto averagePrice = GetAveragePrice(entry.m_position);
      auto transactionReduction = std::min(remainingQuantity,
        Abs(entry.m_position.m_quantity));
      auto grossDelta = -direction * transactionReduction *
        (price - averagePrice);
      auto quantityDelta = direction * transactionReduction;
      auto costBasisDelta = quantityDelta * averagePrice;
      entry.m_grossProfitAndLoss += grossDelta;
      entry.m_position.m_quantity += quantityDelta;
      entry.m_position.m_costBasis += costBasisDelta;
      remainingQuantity -= transactionReduction;
      if(remainingQuantity == 0) {
        total.m_grossProfitAndLoss += entry.m_grossProfitAndLoss;
        total.m_position.m_quantity += Abs(entry.m_position.m_quantity);
        total.m_position.m_costBasis += Abs(entry.m_position.m_costBasis);
        return;
      }
    }
    auto quantityDelta = direction * remainingQuantity;
    auto costBasisDelta = direction * remainingQuantity * price;
    entry.m_position.m_quantity += quantityDelta;
    entry.m_position.m_costBasis += costBasisDelta;
    total.m_grossProfitAndLoss += entry.m_grossProfitAndLoss;
    total.m_position.m_quantity += Abs(entry.m_position.m_quantity);
    total.m_position.m_costBasis += Abs(entry.m_position.m_costBasis);
  }

  template<typename I>
  const typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::GetInventory(const Index& index,
      CurrencyId currency) const {
    auto key = Key(index, currency);
    auto inventoryIterator = m_inventories.find(key);
    if(inventoryIterator == m_inventories.end()) {
      inventoryIterator = m_inventories.insert(
        std::pair(key, Inventory(key))).first;
    }
    return inventoryIterator->second;
  }

  template<typename I>
  const typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::GetTotal(CurrencyId currency) const {
    auto totalsIterator = m_totals.find(currency);
    if(totalsIterator == m_totals.end()) {
      totalsIterator = m_totals.insert(std::pair(currency, Inventory())).first;
      totalsIterator->second.m_position.m_key.m_currency = currency;
    }
    return totalsIterator->second;
  }

  template<typename I>
  Beam::View<std::pair<
      const typename TrueAverageBookkeeper<I>::Key,
      typename TrueAverageBookkeeper<I>::Inventory>>
      TrueAverageBookkeeper<I>::GetInventoryRange() const {
    return Beam::View(m_inventories.begin(), m_inventories.end());
  }

  template<typename I>
  Beam::View<std::pair<const CurrencyId,
      typename TrueAverageBookkeeper<I>::Inventory>>
      TrueAverageBookkeeper<I>::GetTotalsRange() const {
    return Beam::View(m_totals.begin(), m_totals.end());
  }

  template<typename I>
  typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::InternalGetTotal(CurrencyId currency) {
    auto totalsIterator = m_totals.find(currency);
    if(totalsIterator == m_totals.end()) {
      totalsIterator = m_totals.insert(std::pair(currency, Inventory())).first;
      totalsIterator->second.m_position.m_key.m_currency = currency;
    }
    return totalsIterator->second;
  }
}

#endif
