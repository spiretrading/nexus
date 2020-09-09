#ifndef NEXUS_RISK_DATA_STORE_HPP
#define NEXUS_RISK_DATA_STORE_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Concept.hpp>
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Concept used to specify the data store used by the RiskServlet. */
  struct RiskDataStore : Beam::Concept<RiskDataStore> {

    /**
     * Loads an account's InventorySnapshot.
     * @param account The account whose snapshot is to be loaded.
     * @return The <i>account</i>'s InventorySnapshot.
     */
    InventorySnapshot LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account);

    /**
     * Stores an account's InventorySnapshot.
     * @param account The account whose snapshot is being stored.
     * @param snapshot The snapshot to store.
     */
    void Store(const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot);

    void Close();
  };

  /**
   * Strips a snapshot of empty Inventory objects, used to avoid storing empty
   * inventories in a RiskDataStore.
   * @param snapshot The snapshot to trim.
   * @return A copy of the <i>snapshot</i> with all empty Inventory objects
   *         removed.
   */
  inline InventorySnapshot Strip(InventorySnapshot snapshot) {
    snapshot.m_inventories.erase(std::remove_if(snapshot.m_inventories.begin(),
      snapshot.m_inventories.end(), [] (const auto& inventory) {
        return inventory == RiskInventory(inventory.m_position.m_key);
      }), snapshot.m_inventories.end());
    return snapshot;
  }
}

#endif
