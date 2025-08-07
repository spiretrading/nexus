#ifndef NEXUS_LOCAL_RISK_DATA_STORE_HPP
#define NEXUS_LOCAL_RISK_DATA_STORE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::RiskService {

  /** Implements a RiskDataStore in memory. */
  class LocalRiskDataStore {
    public:
      InventorySnapshot load_inventory_snapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);
      void close();

    private:
      Beam::SynchronizedUnorderedMap<
        Beam::ServiceLocator::DirectoryEntry, InventorySnapshot> m_snapshots;
  };

  inline InventorySnapshot LocalRiskDataStore::load_inventory_snapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    if(auto snapshot = m_snapshots.Find(account)) {
      return *snapshot;
    }
    return InventorySnapshot();
  }

  inline void LocalRiskDataStore::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    m_snapshots.Update(account, strip(snapshot));
  }

  inline void LocalRiskDataStore::close() {}
}

#endif
