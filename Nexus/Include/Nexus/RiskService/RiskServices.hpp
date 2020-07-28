#ifndef NEXUS_RISKSERVICES_HPP
#define NEXUS_RISKSERVICES_HPP
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {
namespace RiskService {
  BEAM_DEFINE_RECORD(SecurityValuationUpdate, Security, security,
    RiskSecurityValuation, valuation);
  BEAM_DEFINE_RECORD(InventoryUpdate, Beam::ServiceLocator::DirectoryEntry,
    account, RiskPortfolioInventory, inventory);
  BEAM_DEFINE_RECORD(RiskStateUpdate, Beam::ServiceLocator::DirectoryEntry,
    account, RiskState, risk_state);

  BEAM_DEFINE_SERVICES(RiskServices,

    /*! \interface Nexus::RiskService::SubscribeRiskPortfolioUpdatesService
        \brief Subscribes to the RiskPortfolioUpdates permissioned by the
               session's account.
    */
    //! \cond
    (SubscribeRiskPortfolioUpdatesService,
      "Nexus.RiskService.SubscribeRiskPortfolioUpdatesService",
      std::vector<RiskPortfolioInventoryEntry>));
    //! \endcond

  BEAM_DEFINE_MESSAGES(RiskMessages,

    /*! \interface Nexus::RiskService::SecurityValuationMessage
        \brief Indicates a list of updates to a Security's valuation.
        \param valuations The list of Securities whose valuations have updated.
    */
    //! \cond
    (SecurityValuationMessage, "Nexus.RiskService.SecurityValuationMessage",
      std::vector<SecurityValuationUpdate>, valuations),
    //! \endcond

    /*! \interface Nexus::RiskService::InventoryMessage
        \brief Indicates a list of updates to an account's Inventory.
        \param inventories The list of updated Inventories.
    */
    //! \cond
    (InventoryMessage, "Nexus.RiskService.InventoryMessage",
      std::vector<InventoryUpdate>, inventories),
    //! \endcond

    /*! \interface Nexus::RiskService::RiskStateMessage
        \brief Indicates a list of updates to an account's RiskState.
        \param riskStates The list of updated RiskStates.
    */
    //! \cond
    (RiskStateMessage, "Nexus.RiskService.RiskStateMessage",
      std::vector<RiskStateUpdate>, risk_states));
    //! \endcond
}
}

#endif
