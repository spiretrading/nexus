#ifndef NEXUS_LOCAL_RISK_DATA_STORE_HPP
#define NEXUS_LOCAL_RISK_DATA_STORE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::RiskService {

  /** Implements a RiskDataStore in memory. */
  class LocalRiskDataStore : private boost::noncopyable {
    public:
      ~LocalRiskDataStore();

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);

      void Close();

    private:
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        InventorySnapshot> m_snapshots;
  };

  inline InventorySnapshot LocalRiskDataStore::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto snapshot = m_snapshots.Find(account);
    if(snapshot) {
      return *snapshot;
    }
    return InventorySnapshot();
  }

  inline void LocalRiskDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    m_snapshots.Update(account, Strip(snapshot));
  }

  inline void LocalRiskDataStore::Close() {}
}

#endif
