#ifndef NEXUS_RISK_SERVICES_HPP
#define NEXUS_RISK_SERVICES_HPP
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {
  BEAM_DEFINE_RECORD(SecurityValuationUpdate, (Security, security),
    (SecurityValuation, valuation));
  BEAM_DEFINE_RECORD(InventoryUpdate,
    (Beam::DirectoryEntry, account), (Inventory, inventory));
  BEAM_DEFINE_RECORD(RiskStateUpdate, (Beam::DirectoryEntry, account),
    (RiskState, risk_state));

  /** Standard name for the risk service. */
  inline const auto RISK_SERVICE_NAME = std::string("risk_service");

  BEAM_DEFINE_SERVICES(risk_services,

    /**
     * Loads the InventorySnapshot of a given account.
     * @param account The account to load.
     * @return The <i>account</i>'s InventorySnapshot.
     */
    (LoadInventorySnapshotService,
      "Nexus.RiskService.LoadInventorySnapshotService", InventorySnapshot,
      (Beam::DirectoryEntry, account)),

    /**
     * Resets all inventories whose Security is within a Region.
     * @param region The Region to reset.
     */
    (ResetRegionService, "Nexus.RiskService.ResetRegionService", void,
      (Region, region)),

    /**
     * Subscribes to the RiskPortfolioUpdates permissioned by the session's
     * account.
     */
    (SubscribeRiskPortfolioUpdatesService,
      "Nexus.RiskService.SubscribeRiskPortfolioUpdatesService",
      std::vector<RiskInventoryEntry>));

  BEAM_DEFINE_MESSAGES(risk_messages,

    /**
     * Indicates a list of updates to an account's Inventory.
     * @param inventories The list of updated Inventories.
     */
    (InventoryMessage, "Nexus.RiskService.InventoryMessage",
      (std::vector<InventoryUpdate>, inventories)));
}

#endif
