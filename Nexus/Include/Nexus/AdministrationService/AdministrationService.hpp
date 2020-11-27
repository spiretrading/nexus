#ifndef NEXUS_ADMINISTRATION_SERVICE_HPP
#define NEXUS_ADMINISTRATION_SERVICE_HPP
#include <string>

namespace Nexus::AdministrationService {
  struct AccountIdentity;
  class AccountModificationRequest;
  template<typename B> class AdministrationClient;
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
  class VirtualAdministrationClient;
  template<typename C> class WrapperAdministrationClient;

  /** Standard name for the administration service. */
  inline const std::string SERVICE_NAME = "administration_service";
}

#endif
