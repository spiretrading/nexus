#ifndef NEXUS_MARKETDATAREGISTRYSESSION_HPP
#define NEXUS_MARKETDATAREGISTRYSESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/MarketDataService/EntitlementSet.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketDataRegistrySession
      \brief Stores session info for a MarketDataRegistryServlet.
   */
  class MarketDataRegistrySession :
      public Beam::ServiceLocator::AuthenticatedSession {
    public:

      //! Constructs a MarketDataRegistrySession.
      MarketDataRegistrySession();

      //! Returns the set of market data entitlements.
      EntitlementSet GetEntitlements() const;

      //! Returns the set of market data entitlements.
      EntitlementSet& GetEntitlements();

    private:
      EntitlementSet m_entitlements;
  };

  inline MarketDataRegistrySession::MarketDataRegistrySession() {}

  inline EntitlementSet MarketDataRegistrySession::GetEntitlements() const {
    return m_entitlements;
  }

  inline EntitlementSet& MarketDataRegistrySession::GetEntitlements() {
    return m_entitlements;
  }
}
}

#endif
