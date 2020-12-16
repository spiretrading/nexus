#ifndef NEXUS_ADMINISTRATION_SERVICE_HPP
#define NEXUS_ADMINISTRATION_SERVICE_HPP
#include <string>

namespace Nexus::AdministrationService {
  struct AccountIdentity;
  class AccountModificationRequest;
  template<typename B> class AdministrationClient;
  class AdministrationClientBox;
  class AdministrationDataStore;
  template<typename C, typename S, typename D> class AdministrationServlet;
  class AdministrationSession;
  template<typename D> class CachedAdministrationDataStore;
  class EntitlementModification;
  class LocalAdministrationDataStore;
  class Message;
  class RiskModification;
  template<typename C> class SqlAdministrationDataStore;
  class TradingGroup;

  /** Standard name for the administration service. */
  inline const auto SERVICE_NAME = std::string("administration_service");
}

#endif
