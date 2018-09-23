#ifndef NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#define NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#include <unordered_map>
#include <Beam/Collections/View.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"

namespace Nexus::Accounting {

  /** Implements a Bookkeeper using true average bookkeeping.
      \tparam InventoryType The type of Inventory to manage.
   */
  template<typename InventoryType>
  class TrueAverageBookkeeper {
    public:
      using Inventory = typename Bookkeeper<InventoryType>::Inventory;
      using Index = typename Bookkeeper<InventoryType>::Index;
      using Key = typename Bookkeeper<InventoryType>::Key;

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

  template<typename InventoryType>
  void TrueAverageBookkeeper<InventoryType>::RecordTransaction(
      const Index& index, CurrencyId currency, Quantity quantity,
      Money costBasis, Money fees) {
    Key key(index, currency);
    auto entryIterator = m_inventories.find(key);
    if(entryIterator == m_inventories.end()) {
      entryIterator = m_inventories.insert(
        std::make_pair(key, Inventory(key))).first;
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
    Quantity direction;
    if(quantity < 0) {
      direction = -1;
    } else {
      direction = 1;
    }
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

  template<typename InventoryType>
  const typename TrueAverageBookkeeper<InventoryType>::Inventory&
      TrueAverageBookkeeper<InventoryType>::GetInventory(const Index& index,
      CurrencyId currency) const {
    Key key(index, currency);
    auto inventoryIterator = m_inventories.find(key);
    if(inventoryIterator == m_inventories.end()) {
      inventoryIterator = m_inventories.insert(
        std::make_pair(key, Inventory(key))).first;
    }
    return inventoryIterator->second;
  }

  template<typename InventoryType>
  const typename TrueAverageBookkeeper<InventoryType>::Inventory&
      TrueAverageBookkeeper<InventoryType>::GetTotal(
      CurrencyId currency) const {
    auto totalsIterator = m_totals.find(currency);
    if(totalsIterator == m_totals.end()) {
      totalsIterator = m_totals.insert(
        std::make_pair(currency, Inventory())).first;
      totalsIterator->second.m_position.m_key.m_currency = currency;
    }
    return totalsIterator->second;
  }

  template<typename InventoryType>
  Beam::View<std::pair<
      const typename TrueAverageBookkeeper<InventoryType>::Key,
      typename TrueAverageBookkeeper<InventoryType>::Inventory>>
      TrueAverageBookkeeper<InventoryType>::GetInventoryRange() const {
    return Beam::MakeView(m_inventories.begin(), m_inventories.end());
  }

  template<typename InventoryType>
  Beam::View<std::pair<const CurrencyId,
      typename TrueAverageBookkeeper<InventoryType>::Inventory>>
      TrueAverageBookkeeper<InventoryType>::GetTotalsRange() const {
    return Beam::MakeView(m_totals.begin(), m_totals.end());
  }

  template<typename InventoryType>
  typename TrueAverageBookkeeper<InventoryType>::Inventory&
      TrueAverageBookkeeper<InventoryType>::InternalGetTotal(
      CurrencyId currency) {
    auto totalsIterator = m_totals.find(currency);
    if(totalsIterator == m_totals.end()) {
      totalsIterator = m_totals.insert(
        std::make_pair(currency, Inventory())).first;
      totalsIterator->second.m_position.m_key.m_currency = currency;
    }
    return totalsIterator->second;
  }
}

#endif
