#ifndef NEXUS_COMPLIANCE_HPP
#define NEXUS_COMPLIANCE_HPP
#include <cstdint>
#include <string>

namespace Nexus {
namespace Compliance {
  class ApplicationComplianceClient;
  template<typename MarketDataClientType> class BuyingPowerComplianceRule;
  template<typename TimeClientType> class CancelRestrictionPeriodComplianceRule;
  class ComplianceCheckException;
  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType> class ComplianceCheckOrderExecutionDriver;
  template<typename ServiceProtocolClientBuilderType> class ComplianceClient;
  struct ComplianceParameter;
  class ComplianceRule;
  struct ComplianceRuleDataStore;
  class ComplianceRuleDataStoreException;
  class ComplianceRuleEntry;
  using ComplianceRuleId = std::uint64_t;
  class ComplianceRuleSchema;
  struct ComplianceRuleViolationRecord;
  template<typename ComplianceRuleDataStoreType,
    typename ServiceLocatorClientType> class ComplianceRuleSet;
  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType> class ComplianceServlet;
  class ComplianceSession;
  class LocalComplianceRuleDataStore;
  template<typename KeyType> class MapComplianceRule;
  class MySqlComplianceRuleDataStore;
  template<typename TimeClientType>
    class OpposingOrderCancellationComplianceRule;
  class OrderCountPerSideComplianceRule;
  class PerAccountComplianceRule;
  class RejectCancelsComplianceRule;
  class RejectSubmissionsComplianceRule;
  class SecurityFilterComplianceRule;
  template<typename TimeClientType> class TimeFilterComplianceRule;
  class VirtualComplianceClient;

  // Standard name for the compliance service.
  static const std::string SERVICE_NAME = "compliance_service";
}
}

#endif
