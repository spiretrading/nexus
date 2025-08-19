#ifndef NEXUS_BOOKKEEPER_HPP
#define NEXUS_BOOKKEEPER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Collections/View.hpp>
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus::Accounting {

  /** Performs bookkeeping and cost management of inventories. */
  class Bookkeeper {
    public:

      /**
       * Constructs a Bookkeeper of a specified type using emplacement.
       * @param <T> The type of bookkeeper to emplace.
       * @param args The arguments to pass to the emplaced bookkeeper.
       */
      template<typename T, typename... Args>
      explicit Bookkeeper(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Bookkeeper by copying an existing bookkeeper.
       * @param bookkeeper The bookkeeper to copy.
       */
      template<typename B,
        typename = std::enable_if_t<
          !std::is_same_v<std::remove_cvref_t<B>, Bookkeeper>>>
      explicit Bookkeeper(B&& bookkeeper);

      Bookkeeper(const Bookkeeper& bookkeeper);

      /**
       * Records a transaction.
       * @param security The Security to record the transaction for.
       * @param currency The currency of the transaction.
       * @param quantity The quantity to transact.
       * @param cost_basis The cost basis of the transaction.
       * @param fees Any fees incurred from the transaction.
       */
      void record(const Security& security, CurrencyId currency,
        Quantity quantity, Money cost_basis, Money fees);

      /**
       * Returns the Inventory for a given security and currency.
       * @param security The Security of the Inventory to retrieve.
       * @param currency The currency of the Inventory to retrieve.
       * @return The Inventory for the specified security and currency.
       * @throws std::out_of_range If no inventory is found.
       */
      const Inventory& get_inventory(
        const Security& security, CurrencyId currency) const;

      /**
       * Returns the total Inventory for a given currency.
       * @param currency The currency of the total Inventory to retrieve.
       * @return The total Inventory for the specified currency.
       * @throws std::out_of_range If no inventory is found for the currency.
       */
      const Inventory& get_total(CurrencyId currency) const;

      /** Returns a view of all inventories. */
      Beam::View<const Inventory> get_inventory_range() const;

      /** Returns a view of all total inventories. */
      Beam::View<const Inventory> get_totals_range() const;

    private:
      struct VirtualBookkeeper {
        virtual ~VirtualBookkeeper() = default;
        virtual std::unique_ptr<VirtualBookkeeper> clone() const = 0;
        virtual void record(
          const Security&, CurrencyId, Quantity, Money, Money) = 0;
        virtual const Inventory& get_inventory(
          const Security&, CurrencyId) const = 0;
        virtual const Inventory& get_total(CurrencyId) const = 0;
        virtual Beam::View<const Inventory> get_inventory_range() const = 0;
        virtual Beam::View<const Inventory> get_totals_range() const = 0;
      };
      template<typename B>
      struct WrappedBookkeeper final : VirtualBookkeeper {
        using Bookkeeper = B;
        Beam::GetOptionalLocalPtr<Bookkeeper> m_bookkeeper;

        template<typename... Args>
        WrappedBookkeeper(Args&&... args);
        std::unique_ptr<VirtualBookkeeper> clone() const override;
        void record(const Security& security, CurrencyId currency,
          Quantity quantity, Money cost_basis, Money fees) override;
        const Inventory& get_inventory(
          const Security& security, CurrencyId currency) const override;
        const Inventory& get_total(CurrencyId currency) const override;
        Beam::View<const Inventory> get_inventory_range() const override;
        Beam::View<const Inventory> get_totals_range() const override;
      };
      std::unique_ptr<VirtualBookkeeper> m_bookkeeper;
  };

  /**
   * Concept that evaluates to true if a type is a Bookkeeper instantiation.
   * @param <T> The type to test.
   */
  template<typename T>
  concept IsBookkeeper = std::constructible_from<
    Bookkeeper, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  Bookkeeper::Bookkeeper(std::in_place_type_t<T>, Args&&... args)
    : m_bookkeeper(
        std::make_unique<WrappedBookkeeper<T>>(std::forward<Args>(args)...)) {}

  template<typename B, typename>
  Bookkeeper::Bookkeeper(B&& bookkeeper)
    : Bookkeeper(
        std::in_place_type<std::decay_t<B>>, std::forward<B>(bookkeeper)) {}

  inline Bookkeeper::Bookkeeper(const Bookkeeper& bookkeeper)
    : m_bookkeeper(bookkeeper.m_bookkeeper->clone()) {}

  inline void Bookkeeper::record(const Security& security, CurrencyId currency,
      Quantity quantity, Money cost_basis, Money fees) {
    m_bookkeeper->record(security, currency, quantity, cost_basis, fees);
  }

  inline const Inventory& Bookkeeper::get_inventory(
      const Security& security, CurrencyId currency) const {
    return m_bookkeeper->get_inventory(security, currency);
  }

  inline const Inventory& Bookkeeper::get_total(CurrencyId currency) const {
    return m_bookkeeper->get_total(currency);
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
  std::unique_ptr<typename Bookkeeper::VirtualBookkeeper>
      Bookkeeper::WrappedBookkeeper<B>::clone() const {
    return std::make_unique<WrappedBookkeeper<B>>(*m_bookkeeper);
  }

  template<typename B>
  void Bookkeeper::WrappedBookkeeper<B>::record(const Security& security,
      CurrencyId currency, Quantity quantity, Money cost_basis, Money fees) {
    m_bookkeeper->record(security, currency, quantity, cost_basis, fees);
  }

  template<typename B>
  const Inventory& Bookkeeper::WrappedBookkeeper<B>::get_inventory(
      const Security& security, CurrencyId currency) const {
    return m_bookkeeper->get_inventory(security, currency);
  }

  template<typename B>
  const Inventory& Bookkeeper::WrappedBookkeeper<B>::get_total(
      CurrencyId currency) const {
    return m_bookkeeper->get_total(currency);
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
