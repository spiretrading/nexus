#ifndef NEXUS_COMPLIANCE_RULE_VIOLATION_RECORD_HPP
#define NEXUS_COMPLIANCE_RULE_VIOLATION_RECORD_HPP
#include <string>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::Compliance {

  /** Stores a single compliance rule violation. */
  struct ComplianceRuleViolationRecord {

    /** The account that produced the violation. */
    Beam::ServiceLocator::DirectoryEntry m_account;

    /** The OrderId the violation is attributed to. */
    OrderExecutionService::OrderId m_orderId;

    /** The ComplianceRuleId that was violated. */
    ComplianceRuleId m_ruleId;

    /** The name of the schema that was violated. */
    std::string m_schemaName;

    /** The reason for the violation. */
    std::string m_reason;

    /** The time of the violation. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs a ComplianceRuleViolationRecord. */
    ComplianceRuleViolationRecord();

    /**
     * Constructs a ComplianceRuleViolationRecord used to submit a report.
     * @param account The account that produced the violation.
     * @param orderId The OrderId the violation is attributed to.
     * @param ruleId The ComplianceRuleId that was violated.
     * @param schemaName The name of the rule that was violated.
     * @param reason The reason for the violation.
     */
    ComplianceRuleViolationRecord(Beam::ServiceLocator::DirectoryEntry account,
      OrderExecutionService::OrderId orderId, ComplianceRuleId ruleId,
      std::string schemaName, std::string reason);

    /**
     * Constructs a fully specified ComplianceRuleViolationRecord.
     * @param account The account that produced the violation.
     * @param orderId The OrderId the violation is attributed to.
     * @param ruleId The ComplianceRuleId that was violated.
     * @param schemaName The name of the rule that was violated.
     * @param reason The reason for the violation.
     */
    ComplianceRuleViolationRecord(Beam::ServiceLocator::DirectoryEntry account,
      OrderExecutionService::OrderId orderId, ComplianceRuleId ruleId,
      std::string schemaName, std::string reason,
      boost::posix_time::ptime timestamp);
  };

  inline ComplianceRuleViolationRecord::ComplianceRuleViolationRecord()
    : m_orderId(0),
      m_ruleId(0) {}

  inline ComplianceRuleViolationRecord::ComplianceRuleViolationRecord(
    Beam::ServiceLocator::DirectoryEntry account,
    OrderExecutionService::OrderId orderId, ComplianceRuleId ruleId,
    std::string schemaName, std::string reason)
    : m_account(std::move(account)),
      m_orderId(std::move(orderId)),
      m_ruleId(std::move(ruleId)),
      m_schemaName(std::move(schemaName)),
      m_reason(std::move(reason)) {}

  inline ComplianceRuleViolationRecord::ComplianceRuleViolationRecord(
    Beam::ServiceLocator::DirectoryEntry account,
    OrderExecutionService::OrderId orderId, ComplianceRuleId ruleId,
    std::string schemaName, std::string reason,
    boost::posix_time::ptime timestamp)
    : m_account(std::move(account)),
      m_orderId(std::move(orderId)),
      m_ruleId(std::move(ruleId)),
      m_schemaName(std::move(schemaName)),
      m_reason(std::move(reason)),
      m_timestamp(std::move(timestamp)) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Compliance::ComplianceRuleViolationRecord> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Compliance::ComplianceRuleViolationRecord& value,
        unsigned int version) {
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("order_id", value.m_orderId);
      shuttle.Shuttle("rule_id", value.m_ruleId);
      shuttle.Shuttle("schema_name", value.m_schemaName);
      shuttle.Shuttle("reason", value.m_reason);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
