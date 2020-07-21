#ifndef NEXUS_RISK_SERVICE_HPP
#define NEXUS_RISK_SERVICE_HPP
#include <string>

namespace Nexus::RiskService {
  struct AccountRecord;
  struct PositionSnapshot;
  template<typename B> class RiskClient;
  struct RiskDataStore;
  struct RiskParameters;
  struct RiskPositionUpdate;
  struct RiskProfitAndLossUpdate;
  template<typename C, typename A, typename O, typename R> class RiskServlet;
  class RiskSession;
  struct RiskState;
  template<typename R, typename A, typename M, typename T, typename C>
    class RiskStateMonitor;
  template<typename P, typename T> class RiskStateTracker;
  struct RiskStateUpdate;
  template<typename A, typename O> class RiskTransitionController;
  template<typename O> class RiskTransitionTracker;
  template<typename C> class SqlRiskDataStore;
  class VirtualRiskClient;
  template<typename C> class WrapperRiskClient;

  /** Standard name for the risk service. */
  inline const std::string SERVICE_NAME = "risk_service";
}

#endif
