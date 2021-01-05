#ifndef NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#define NEXUS_TRUE_AVERAGE_BOOKKEEPER_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/View.hpp>
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

      void RecordTransaction(const Index& index, CurrencyId currency,
        Quantity quantity, Money costBasis, Money fees);

      const Inventory& GetInventory(const Index& index,
        CurrencyId currency) const;

      const Inventory& GetTotal(CurrencyId currency) const;

      Beam::View<const Inventory> GetInventoryRange() const;

      Beam::View<const Inventory> GetTotalsRange() const;

    private:
      std::unordered_map<Key, Inventory> m_inventories;
      std::unordered_map<CurrencyId, Inventory> m_totals;

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
      entryIterator =
        m_inventories.insert(std::pair(key, Inventory(key))).first;
    }
    auto& entry = entryIterator->second;
    entry.m_fees += fees;
    auto& total = InternalGetTotal(currency);
    total.m_fees += fees;
    if(quantity == 0) {
      return;
    }
    entry.m_volume += Abs(quantity);
    ++entry.m_transactionCount;
    total.m_volume += Abs(quantity);
    ++total.m_transactionCount;
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
      auto transactionReduction =
        std::min(remainingQuantity, Abs(entry.m_position.m_quantity));
      auto averagePrice = GetAveragePrice(entry.m_position);
      entry.m_grossProfitAndLoss +=
        -direction * transactionReduction * (price - averagePrice);
      auto quantityDelta = direction * transactionReduction;
      entry.m_position.m_quantity += quantityDelta;
      if(entry.m_position.m_quantity == 0) {
        entry.m_position.m_costBasis = Money::ZERO;
      } else {
        entry.m_position.m_costBasis += quantityDelta * averagePrice;
      }
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
      static auto emptyInventories =
        Beam::SynchronizedUnorderedMap<Key, Inventory>();
      return emptyInventories.GetOrInsert(key, [&] {
        return Inventory(key);
      });
    }
    return inventoryIterator->second;
  }

  template<typename I>
  const typename TrueAverageBookkeeper<I>::Inventory&
      TrueAverageBookkeeper<I>::GetTotal(CurrencyId currency) const {
    auto totalsIterator = m_totals.find(currency);
    if(totalsIterator == m_totals.end()) {
      static auto emptyTotals =
        Beam::SynchronizedUnorderedMap<CurrencyId, Inventory>();
      return emptyTotals.GetOrInsert(currency, [&] {
        auto totals = Inventory();
        totals.m_position.m_key.m_currency = currency;
        return totals;
      });
    }
    return totalsIterator->second;
  }

  template<typename I>
  Beam::View<const typename TrueAverageBookkeeper<I>::Inventory>
      TrueAverageBookkeeper<I>::GetInventoryRange() const {
    using Accessor = Details::ValueAccessor<std::pair<const Key, Inventory>>;
    return Beam::View(
      boost::make_transform_iterator(m_inventories.begin(), Accessor()),
      boost::make_transform_iterator(m_inventories.end(), Accessor()));
  }

  template<typename I>
  Beam::View<const typename TrueAverageBookkeeper<I>::Inventory>
      TrueAverageBookkeeper<I>::GetTotalsRange() const {
    using Accessor =
      Details::ValueAccessor<std::pair<const CurrencyId, Inventory>>;
    return Beam::View(
      boost::make_transform_iterator(m_totals.begin(), Accessor()),
      boost::make_transform_iterator(m_totals.end(), Accessor()));
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
