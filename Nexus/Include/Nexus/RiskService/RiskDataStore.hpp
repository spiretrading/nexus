#ifndef NEXUS_RISK_DATA_STORE_HPP
#define NEXUS_RISK_DATA_STORE_HPP
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Concept.hpp>
#include "Nexus/RiskService/PositionSnapshot.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::RiskService {

  /** Concept used to specify the data store used by the RiskServlet. */
  struct RiskDataStore : Beam::Concept<RiskDataStore> {

    /**
     * Loads an account's PositionSnapshot.
     * @param account The account whose snapshot is to be loaded.
     * @return The <i>account</i>'s PositionSnapshot.
     */
    PositionSnapshot LoadPositionSnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account);

    /**
     * Stores an account's PositionSnapshot.
     * @param account The account whose snapshot is being stored.
     * @param snapshot The snapshot to store.
     */
    void Store(const Beam::ServiceLocator::DirectoryEntry& account,
      const PositionSnapshot& snapshot);

    void Open();

    void Close();
  };
}

#endif