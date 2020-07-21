#ifndef NEXUS_LOCAL_RISK_DATA_STORE_HPP
#define NEXUS_LOCAL_RISK_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
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

      void Open();

      void Close();

    private:
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        InventorySnapshot> m_snapshots;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline LocalRiskDataStore::~LocalRiskDataStore() {
    Close();
  }

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
    m_snapshots.Update(account, snapshot);
  }

  inline void LocalRiskDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalRiskDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void LocalRiskDataStore::Shutdown() {
    m_openState.SetClosed();
  }
}

#endif
