#ifndef NEXUS_BOOKKEEPER_HPP
#define NEXUS_BOOKKEEPER_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Collections/View.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include "Nexus/Accounting/Inventory.hpp"

namespace Nexus {

  /** Checks if a type implements a Bookkeeper. */
  template<typename T>
  concept IsBookkeeper = requires(T& bookkeeper) {
    bookkeeper.record(std::declval<const Ticker&>(), std::declval<Asset>(),
      std::declval<Quantity>(), std::declval<Money>(), std::declval<Money>());
    { bookkeeper.get_inventory(std::declval<const Ticker&>()) } ->
      std::same_as<const Inventory&>;
    { bookkeeper.get_total(std::declval<Asset>()) } ->
      std::same_as<const Inventory&>;
    { bookkeeper.get_inventory_range() } ->
        std::same_as<Beam::View<const Inventory>>;
    { bookkeeper.get_totals_range() } ->
        std::same_as<Beam::View<const Inventory>>;
  };

  /** Performs bookkeeping and cost management of inventories. */
  class Bookkeeper {
    public:

      /**
       * Constructs a Bookkeeper of a specified type using emplacement.
       * @tparam T The type of bookkeeper to emplace.
       * @param args The arguments to pass to the emplaced bookkeeper.
       */
      template<IsBookkeeper T, typename... Args>
      explicit Bookkeeper(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Bookkeeper by referencing an existing bookkeeper.
       * @param bookkeeper The bookkeeper to reference.
       */
      template<Beam::DisableCopy<Bookkeeper> T> requires
        IsBookkeeper<Beam::dereference_t<T>>
      Bookkeeper(T&& bookkeeper);

      Bookkeeper(const Bookkeeper&) = default;
      Bookkeeper(Bookkeeper&&) = default;

      /**
       * Records a transaction.
       * @param ticker The ticker to record the transaction for.
       * @param currency The currency used to value the transaction.
       * @param quantity The quantity to transact.
       * @param cost_basis The cost basis of the transaction.
       * @param fees Any fees incurred from the transaction.
       */
      void record(const Ticker& ticker, Asset currency, Quantity quantity,
        Money cost_basis, Money fees);

      /**
       * Returns the Inventory for a given ticker.
       * @param ticker The Ticker of the Inventory to retrieve.
       * @return The Inventory for the specified ticker.
       * @throws std::out_of_range If no inventory is found.
       */
      const Inventory& get_inventory(const Ticker& ticker) const;

      /**
       * Returns the total Inventory for a given asset.
       * @param asset The asset of the total Inventory to retrieve.
       * @return The total Inventory for the specified asset.
       * @throws std::out_of_range If no inventory is found for the asset.
       */
      const Inventory& get_total(Asset asset) const;

      /** Returns a view of all inventories. */
      Beam::View<const Inventory> get_inventory_range() const;

      /** Returns a view of all total inventories. */
      Beam::View<const Inventory> get_totals_range() const;

    private:
      struct VirtualBookkeeper {
        virtual ~VirtualBookkeeper() = default;

        virtual void record(const Ticker&, Asset, Quantity, Money, Money) = 0;
        virtual const Inventory& get_inventory(const Ticker&) const = 0;
        virtual const Inventory& get_total(Asset) const = 0;
        virtual Beam::View<const Inventory> get_inventory_range() const = 0;
        virtual Beam::View<const Inventory> get_totals_range() const = 0;
      };
      template<typename B>
      struct WrappedBookkeeper final : VirtualBookkeeper {
        using Bookkeeper = B;
        Beam::local_ptr_t<Bookkeeper> m_bookkeeper;

        template<typename... Args>
        WrappedBookkeeper(Args&&... args);

        void record(const Ticker& ticker, Asset currency, Quantity quantity,
          Money cost_basis, Money fees) override;
        const Inventory& get_inventory(const Ticker& ticker) const override;
        const Inventory& get_total(Asset asset) const override;
        Beam::View<const Inventory> get_inventory_range() const override;
        Beam::View<const Inventory> get_totals_range() const override;
      };
      Beam::VirtualPtr<VirtualBookkeeper> m_bookkeeper;
  };

  template<IsBookkeeper T, typename... Args>
  Bookkeeper::Bookkeeper(std::in_place_type_t<T>, Args&&... args)
    : m_bookkeeper(Beam::make_virtual_ptr<WrappedBookkeeper<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<Bookkeeper> T> requires
    IsBookkeeper<Beam::dereference_t<T>>
  Bookkeeper::Bookkeeper(T&& bookkeeper)
    : m_bookkeeper(Beam::make_virtual_ptr<WrappedBookkeeper<
        std::remove_cvref_t<T>>>(std::forward<T>(bookkeeper))) {}

  inline void Bookkeeper::record(const Ticker& ticker, Asset currency,
      Quantity quantity, Money cost_basis, Money fees) {
    m_bookkeeper->record(ticker, currency, quantity, cost_basis, fees);
  }

  inline const Inventory& Bookkeeper::get_inventory(
      const Ticker& ticker) const {
    return m_bookkeeper->get_inventory(ticker);
  }

  inline const Inventory& Bookkeeper::get_total(Asset asset) const {
    return m_bookkeeper->get_total(asset);
  }

  inline Beam::View<const Inventory> Bookkeeper::get_inventory_range() const {
    return m_bookkeeper->get_inventory_range();
  }

  inline Beam::View<const Inventory> Bookkeeper::get_totals_range() const {
    return m_bookkeeper->get_totals_range();
  }

  template<typename B>
  template<typename... Args>
  Bookkeeper::WrappedBookkeeper<B>::WrappedBookkeeper(Args&&... args)
    : m_bookkeeper(std::forward<Args>(args)...) {}

  template<typename B>
  void Bookkeeper::WrappedBookkeeper<B>::record(const Ticker& ticker,
      Asset currency, Quantity quantity, Money cost_basis, Money fees) {
    m_bookkeeper->record(ticker, currency, quantity, cost_basis, fees);
  }

  template<typename B>
  const Inventory& Bookkeeper::WrappedBookkeeper<B>::get_inventory(
      const Ticker& ticker) const {
    return m_bookkeeper->get_inventory(ticker);
  }

  template<typename B>
  const Inventory& Bookkeeper::WrappedBookkeeper<B>::get_total(
      Asset asset) const {
    return m_bookkeeper->get_total(asset);
  }

  template<typename B>
  Beam::View<const Inventory>
      Bookkeeper::WrappedBookkeeper<B>::get_inventory_range() const {
    return m_bookkeeper->get_inventory_range();
  }

  template<typename B>
  Beam::View<const Inventory>
      Bookkeeper::WrappedBookkeeper<B>::get_totals_range() const {
    return m_bookkeeper->get_totals_range();
  }
}

#endif
