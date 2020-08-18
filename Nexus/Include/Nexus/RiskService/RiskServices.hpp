#ifndef NEXUS_RISK_SERVICES_HPP
#define NEXUS_RISK_SERVICES_HPP
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
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

  BEAM_DEFINE_SERVICES(RiskServices,

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
