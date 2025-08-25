#ifndef NEXUS_RISK_DATA_STORE_HPP
#define NEXUS_RISK_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/RiskService/InventorySnapshot.hpp"

namespace Nexus {

  /** Concept used to specify the data store used by the RiskServlet. */
  class RiskDataStore {
    public:

      /**
       * Constructs a RiskDataStore of a specified type using emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit RiskDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a RiskDataStore by copying an existing data store.
       * @param data_store The data store to copy.
       */
      template<typename D>
      explicit RiskDataStore(D data_store);

      explicit RiskDataStore(RiskDataStore* data_store);

      explicit RiskDataStore(const std::shared_ptr<RiskDataStore>& data_store);

      explicit RiskDataStore(const std::unique_ptr<RiskDataStore>& data_store);

      /**
       * Loads an account's InventorySnapshot.
       * @param account The account whose snapshot is to be loaded.
       * @return The <i>account</i>'s InventorySnapshot.
       */
      InventorySnapshot load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Stores an account's InventorySnapshot.
       * @param account The account whose snapshot is being stored.
       * @param snapshot The snapshot to store.
       */
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);

      void close();

    private:
      struct VirtualRiskDataStore {
        virtual ~VirtualRiskDataStore() = default;
        virtual InventorySnapshot load_inventory_snapshot(
          const Beam::ServiceLocator::DirectoryEntry&) = 0;
        virtual void store(const Beam::ServiceLocator::DirectoryEntry& account,
          const InventorySnapshot&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedRiskDataStore final : VirtualRiskDataStore {
        using RiskDataStoreType = D;
        Beam::GetOptionalLocalPtr<RiskDataStoreType> m_data_store;

        template<typename... Args>
        WrappedRiskDataStore(Args&&... args);
        InventorySnapshot load_inventory_snapshot(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        void store(const Beam::ServiceLocator::DirectoryEntry& account,
          const InventorySnapshot& snapshot) override;
        void close() override;
      };
      std::shared_ptr<VirtualRiskDataStore> m_data_store;
  };

  /** Checks if a type implements a RiskDataStore. */
  template<typename T>
  concept IsRiskDataStore = std::constructible_from<
    RiskDataStore, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  RiskDataStore::RiskDataStore(std::in_place_type_t<T>, Args&&... args)
    : m_data_store(std::make_shared<WrappedRiskDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename D>
  RiskDataStore::RiskDataStore(D data_store)
    : RiskDataStore(std::in_place_type<D>, std::move(data_store)) {}

  inline RiskDataStore::RiskDataStore(RiskDataStore* data_store)
    : RiskDataStore(*data_store) {}

  inline RiskDataStore::RiskDataStore(
    const std::shared_ptr<RiskDataStore>& data_store)
    : RiskDataStore(*data_store) {}

  inline RiskDataStore::RiskDataStore(
    const std::unique_ptr<RiskDataStore>& data_store)
    : RiskDataStore(*data_store) {}

  inline InventorySnapshot RiskDataStore::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_data_store->load_inventory_snapshot(account);
  }

  inline void RiskDataStore::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
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
      load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_data_store->load_inventory_snapshot(account);
  }

  template<typename D>
  void RiskDataStore::WrappedRiskDataStore<D>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    m_data_store->store(account, snapshot);
  }

  template<typename D>
  void RiskDataStore::WrappedRiskDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
