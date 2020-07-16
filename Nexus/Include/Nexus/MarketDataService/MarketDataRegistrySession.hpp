#ifndef NEXUS_MARKET_DATA_REGISTRY_SESSION_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /** Stores session info for a MarketDataRegistryServlet. */
  class MarketDataRegistrySession :
      public Beam::ServiceLocator::AuthenticatedSession {
    public:

      /** The session's roles. */
      AdministrationService::AccountRoles m_roles;

      /** The entitlements granted to the session. */
      EntitlementSet m_entitlements;
  };
}

#endif
