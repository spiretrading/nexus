#ifndef NEXUS_RISK_DATA_STORE_HPP
#define NEXUS_RISK_DATA_STORE_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/RiskService/InventorySnapshot.hpp"

namespace Nexus {

  /** Concept for types that can be used as a risk data store. */
  template<typename T>
  concept IsRiskDataStore = Beam::IsConnection<T> && requires(T& store) {
    { store.load_inventory_snapshot(
        std::declval<const Beam::DirectoryEntry&>()) } ->
          std::same_as<InventorySnapshot>;
    store.store(std::declval<const Beam::DirectoryEntry&>(),
      std::declval<const InventorySnapshot&>());
  };

  /** Provides a generic interface over an arbitrary RiskDataStore. */
  class RiskDataStore {
    public:

      /**
       * Constructs a RiskDataStore of a specified type using emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsRiskDataStore T, typename... Args>
      explicit RiskDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a RiskDataStore by referencing an existing data store.
       * @param data_store The data store to reference.
       */
      template<Beam::DisableCopy<RiskDataStore> T> requires
        IsRiskDataStore<Beam::dereference_t<T>>
      RiskDataStore(T&& data_store);

      RiskDataStore(const RiskDataStore&) = default;
      RiskDataStore(RiskDataStore&&) = default;

      /**
       * Loads an account's InventorySnapshot.
       * @param account The account whose snapshot is to be loaded.
       * @return The account's InventorySnapshot.
       */
      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);

      /**
       * Stores an account's InventorySnapshot.
       * @param account The account whose snapshot is being stored.
       * @param snapshot The snapshot to store.
       */
      void store(const Beam::DirectoryEntry& account,
        const InventorySnapshot& snapshot);

      /** Closes the data store. */
      void close();

    private:
      struct VirtualRiskDataStore {
        virtual ~VirtualRiskDataStore() = default;

        virtual InventorySnapshot load_inventory_snapshot(
          const Beam::DirectoryEntry& account) = 0;
        virtual void store(const Beam::DirectoryEntry& account,
          const InventorySnapshot& snapshot) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedRiskDataStore final : VirtualRiskDataStore {
        using DataStore = D;
        Beam::local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedRiskDataStore(Args&&... args);

        InventorySnapshot load_inventory_snapshot(
          const Beam::DirectoryEntry& account) override;
        void store(const Beam::DirectoryEntry& account,
          const InventorySnapshot& snapshot) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualRiskDataStore> m_data_store;
  };

  template<IsRiskDataStore T, typename... Args>
  RiskDataStore::RiskDataStore(std::in_place_type_t<T>, Args&&... args)
    : m_data_store(Beam::make_virtual_ptr<WrappedRiskDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<RiskDataStore> T> requires
    IsRiskDataStore<Beam::dereference_t<T>>
  RiskDataStore::RiskDataStore(T&& data_store)
    : m_data_store(Beam::make_virtual_ptr<WrappedRiskDataStore<
        std::remove_cvref_t<T>>>(std::forward<T>(data_store))) {}

  inline InventorySnapshot RiskDataStore::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    return m_data_store->load_inventory_snapshot(account);
  }

  inline void RiskDataStore::store(
      const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot) {
    m_data_store->store(account, snapshot);
  }

  inline void RiskDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  RiskDataStore::WrappedRiskDataStore<D>::WrappedRiskDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  InventorySnapshot RiskDataStore::WrappedRiskDataStore<D>::
      load_inventory_snapshot(const Beam::DirectoryEntry& account) {
    return m_data_store->load_inventory_snapshot(account);
  }

  template<typename D>
  void RiskDataStore::WrappedRiskDataStore<D>::store(
      const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot) {
    m_data_store->store(account, snapshot);
  }

  template<typename D>
  void RiskDataStore::WrappedRiskDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
