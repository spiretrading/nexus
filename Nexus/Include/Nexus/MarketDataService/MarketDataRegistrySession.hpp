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

  /**
   * Tests if a session has been granted a market data entitlement.
   * @param session The session to test.
   * @param key The entitlement to check.
   * @param type The type of market data to test.
   * @return <code>true</code> iff the session has been granted the entitlement.
   */
  inline bool HasEntitlement(const MarketDataRegistrySession& session,
      const EntitlementKey& key, MarketDataType type) {
    return session.m_roles.Test(AdministrationService::AccountRole::SERVICE) ||
      session.m_roles.Test(AdministrationService::AccountRole::ADMINISTRATOR) ||
      session.m_entitlements.HasEntitlement(key, type);
  }

  /**
   * Tests if a session has been granted a market data entitlement for a query.
   * @param session The session to test.
   * @param query The market data being queried.
   * @return <code>true</code> iff the session has been granted the entitlement.
   */
  template<typename T>
  bool HasEntitlement(const MarketDataRegistrySession& session,
      const SecurityMarketDataQuery& query) {
    return session.m_roles.Test(AdministrationService::AccountRole::SERVICE) ||
      session.m_roles.Test(AdministrationService::AccountRole::ADMINISTRATOR) ||
      HasEntitlement<T>(session.m_entitlements, query);
  }

  /**
   * Tests if a session has been granted a market data entitlement for a query.
   * @param session The session to test.
   * @param query The market data being queried.
   * @return <code>true</code> iff the session has been granted the entitlement.
   */
  template<typename T>
  bool HasEntitlement(const MarketDataRegistrySession& session,
      const MarketWideDataQuery& query) {
    return session.m_roles.Test(AdministrationService::AccountRole::SERVICE) ||
      session.m_roles.Test(AdministrationService::AccountRole::ADMINISTRATOR) ||
      HasEntitlement<T>(session.m_entitlements, query);
  }
}

#endif
