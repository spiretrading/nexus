#ifndef NEXUS_LOCAL_RISK_DATA_STORE_HPP
#define NEXUS_LOCAL_RISK_DATA_STORE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus {

  /** Implements a RiskDataStore in memory. */
  class LocalRiskDataStore {
    public:
      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account,
        const InventorySnapshot& snapshot);
      void close();

    private:
      Beam::SynchronizedUnorderedMap<
        Beam::DirectoryEntry, InventorySnapshot> m_snapshots;
  };

  inline InventorySnapshot LocalRiskDataStore::load_inventory_snapshot(
      const Beam::DirectoryEntry& account) {
    return m_snapshots.try_load(account).value_or_eval([] {
      return InventorySnapshot();
    });
  }

  inline void LocalRiskDataStore::store(
      const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot) {
    m_snapshots.update(account, strip(snapshot));
  }

  inline void LocalRiskDataStore::close() {}
}

#endif
