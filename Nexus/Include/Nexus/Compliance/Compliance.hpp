#ifndef NEXUS_COMPLIANCE_HPP
#define NEXUS_COMPLIANCE_HPP
#include <cstdint>
#include <string>

namespace Nexus::Compliance {
  class ApplicationComplianceClient;
  template<typename MarketDataClientType> class BuyingPowerComplianceRule;
  template<typename DataStoreType> class CachedComplianceRuleDataStore;
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
  template<typename TimeClientType>
    class OpposingOrderCancellationComplianceRule;
  template<typename TimeClientType> class OpposingOrderSubmissionComplianceRule;
  class OrderCountPerSideComplianceRule;
  class PerAccountComplianceRule;
  class RejectCancelsComplianceRule;
  class RejectSubmissionsComplianceRule;
  class SecurityFilterComplianceRule;
  template<typename C> class SqlComplianceRuleDataStore;
  template<typename TimeClientType>
    class SubmissionRestrictionPeriodComplianceRule;
  template<typename TimeClientType> class TimeFilterComplianceRule;
  class VirtualComplianceClient;

  // Standard name for the compliance service.
  inline const std::string SERVICE_NAME = "compliance_service";
}

#endif
