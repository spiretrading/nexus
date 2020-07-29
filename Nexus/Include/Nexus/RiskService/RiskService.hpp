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
  template<typename P, typename T> class RiskStateCalculator;
  struct RiskStateUpdate;
  template<typename O> class RiskTransitionCalculator;
  template<typename C> class SqlRiskDataStore;
  class VirtualRiskClient;
  class VirtualRiskDataStore;
  template<typename C> class WrapperRiskClient;
  template<typename D> class WrapperRiskDataStore;

  /** Standard name for the risk service. */
  inline const std::string SERVICE_NAME = "risk_service";
}

#endif
