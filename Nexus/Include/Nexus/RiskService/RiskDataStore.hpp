#ifndef NEXUS_RISK_DATA_STORE_HPP
#define NEXUS_RISK_DATA_STORE_HPP
#include <memory>
#include <utility>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/RiskService/InventorySnapshot.hpp"

namespace Nexus::RiskService {

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
  };
}

#endif
