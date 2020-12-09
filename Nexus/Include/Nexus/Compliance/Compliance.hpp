#ifndef NEXUS_COMPLIANCE_HPP
#define NEXUS_COMPLIANCE_HPP
#include <cstdint>
#include <string>

namespace Nexus::Compliance {
  template<typename C> class BuyingPowerComplianceRule;
  template<typename D> class CachedComplianceRuleDataStore;
  template<typename C> class CancelRestrictionPeriodComplianceRule;
  class ComplianceCheckException;
  template<typename D, typename C, typename S>
    class ComplianceCheckOrderExecutionDriver;
  template<typename B> class ComplianceClient;
  class ComplianceClientBox;
  struct ComplianceParameter;
  class ComplianceRule;
  struct ComplianceRuleDataStore;
  class ComplianceRuleDataStoreException;
  class ComplianceRuleEntry;
  using ComplianceRuleId = std::uint64_t;
  class ComplianceRuleSchema;
  struct ComplianceRuleViolationRecord;
  template<typename C, typename S> class ComplianceRuleSet;
  struct ComplianceRuleViolationRecord;
  template<typename C, typename S, typename A, typename D, typename T>
    class ComplianceServlet;
  class ComplianceSession;
  class LocalComplianceRuleDataStore;
  template<typename K> class MapComplianceRule;
  template<typename C> class OpposingOrderCancellationComplianceRule;
  template<typename C> class OpposingOrderSubmissionComplianceRule;
  class OrderCountPerSideComplianceRule;
  class PerAccountComplianceRule;
  class RejectCancelsComplianceRule;
  class RejectSubmissionsComplianceRule;
  class SecurityFilterComplianceRule;
  template<typename C> class SqlComplianceRuleDataStore;
  template<typename C> class SubmissionRestrictionPeriodComplianceRule;
  template<typename C> class TimeFilterComplianceRule;

  /** Standard name for the compliance service. */
  inline const auto SERVICE_NAME = std::string("compliance_service");
}

#endif
