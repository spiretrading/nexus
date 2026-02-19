#ifndef NEXUS_MARKET_DATA_REGISTRY_SESSION_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"

namespace Nexus {

  /** Stores session info for a MarketDataRegistryServlet. */
  class MarketDataRegistrySession : public Beam::AuthenticatedSession {
    public:

      /** The session's roles. */
      AccountRoles m_roles;

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
  inline bool has_entitlement(const MarketDataRegistrySession& session,
      const EntitlementKey& key, MarketDataType type) {
    return session.m_roles.test(AccountRole::SERVICE) ||
      session.m_roles.test(AccountRole::ADMINISTRATOR) ||
        session.m_entitlements.contains(key, type);
  }

  /**
   * Tests if a session has been granted a market data entitlement for a query.
   * @param session The session to test.
   * @param query The market data being queried.
   * @return <code>true</code> iff the session has been granted the entitlement.
   */
  template<typename T>
  bool has_entitlement(
      const MarketDataRegistrySession& session, const TickerQuery& query) {
    return session.m_roles.test(AccountRole::SERVICE) ||
      session.m_roles.test(AccountRole::ADMINISTRATOR) ||
        contains<T>(session.m_entitlements, query);
  }

  /**
   * Tests if a session has been granted a market data entitlement for a query.
   * @param session The session to test.
   * @param query The market data being queried.
   * @return <code>true</code> iff the session has been granted the entitlement.
   */
  template<typename T>
  bool has_entitlement(const MarketDataRegistrySession& session,
      const VenueMarketDataQuery& query) {
    return session.m_roles.test(AccountRole::SERVICE) ||
      session.m_roles.test(AccountRole::ADMINISTRATOR) ||
        contains<T>(session.m_entitlements, query);
  }
}

#endif
