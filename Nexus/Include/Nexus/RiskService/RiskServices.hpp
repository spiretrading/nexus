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
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::RiskService {
  BEAM_DEFINE_RECORD(SecurityValuationUpdate, Security, security,
    RiskSecurityValuation, valuation);
  BEAM_DEFINE_RECORD(InventoryUpdate, Beam::ServiceLocator::DirectoryEntry,
    account, RiskInventory, inventory);
  BEAM_DEFINE_RECORD(RiskStateUpdate, Beam::ServiceLocator::DirectoryEntry,
    account, RiskState, risk_state);

  /** Standard name for the risk service. */
  inline const std::string SERVICE_NAME = "risk_service";

  BEAM_DEFINE_SERVICES(RiskServices,

    /**
     * Loads the InventorySnapshot of a given account.
     * @param account The account to load.
     * @return The <i>account</i>'s InventorySnapshot.
     */
    (LoadInventorySnapshotService,
      "Nexus.RiskService.LoadInventorySnapshotService", InventorySnapshot,
      Beam::ServiceLocator::DirectoryEntry, account),

    /**
     * Resets all inventories whose Security is within a Region.
     * @param region The Region to reset.
     */
    (ResetRegionService, "Nexus.RiskService.ResetRegionService", void, Region,
      region),

    /**
     * Subscribes to the RiskPortfolioUpdates permissioned by the session's
     * account.
     */
    (SubscribeRiskPortfolioUpdatesService,
      "Nexus.RiskService.SubscribeRiskPortfolioUpdatesService",
      std::vector<RiskInventoryEntry>));

  BEAM_DEFINE_MESSAGES(RiskMessages,

    /**
     * Indicates a list of updates to a Security's valuation.
     * @param valuations The list of Securities whose valuations have updated.
     */
    (SecurityValuationMessage, "Nexus.RiskService.SecurityValuationMessage",
      std::vector<SecurityValuationUpdate>, valuations),

    /**
     * Indicates a list of updates to an account's Inventory.
     * @param inventories The list of updated Inventories.
     */
    (InventoryMessage, "Nexus.RiskService.InventoryMessage",
      std::vector<InventoryUpdate>, inventories),

    /**
     * Indicates a list of updates to an account's RiskState.
     * @param riskStates The list of updated RiskStates.
     */
    (RiskStateMessage, "Nexus.RiskService.RiskStateMessage",
      std::vector<RiskStateUpdate>, risk_states));
}

#endif
