#ifndef NEXUS_RISK_SERVICE_HPP
#define NEXUS_RISK_SERVICE_HPP
#include <string>

namespace Nexus::RiskService {
  template<typename A, typename M, typename O, typename R, typename T,
    typename D> class ConsolidatedRiskController;
  struct InventorySnapshot;
  template<typename B> class RiskClient;
  class RiskClientBox;
  template<typename A, typename M, typename O, typename R, typename T,
    typename D> class RiskController;
  struct RiskDataStore;
  struct RiskParameters;
  struct RiskPositionUpdate;
  struct RiskProfitAndLossUpdate;
  template<typename C, typename A, typename M, typename O, typename R,
    typename T, typename D> class RiskServlet;
  class RiskSession;
  struct RiskState;
  template<typename T> class RiskStateModel;
  struct RiskStateUpdate;
  template<typename O> class RiskTransitionModel;
  template<typename C> class SqlRiskDataStore;
  class TestRiskDataStore;
  class VirtualRiskDataStore;
  template<typename D> class WrapperRiskDataStore;

  /** Standard name for the risk service. */
  inline const std::string SERVICE_NAME = "risk_service";
}

#endif
