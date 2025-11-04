#ifndef NEXUS_COMPLIANCE_RULE_VIOLATION_RECORD_HPP
#define NEXUS_COMPLIANCE_RULE_VIOLATION_RECORD_HPP
#include <ostream>
#include <string>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

namespace Nexus {

  /** Stores a single compliance rule violation. */
  struct ComplianceRuleViolationRecord {

    /** The account that produced the violation. */
    Beam::DirectoryEntry m_account;

    /** The OrderId the violation is attributed to. */
    OrderId m_order_id = 0;

    /** The ComplianceRuleId that was violated. */
    ComplianceRuleEntry::Id m_rule_id = 0;

    /** The name of the schema that was violated. */
    std::string m_schema_name;

    /** The reason for the violation. */
    std::string m_reason;

    /** The time of the violation. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const ComplianceRuleViolationRecord&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ComplianceRuleViolationRecord& record) {
    return out << '(' << record.m_account << ' ' << record.m_order_id << ' ' <<
      record.m_rule_id << ' ' << record.m_schema_name << " \"" <<
      record.m_reason << "\" " << record.m_timestamp << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::ComplianceRuleViolationRecord> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::ComplianceRuleViolationRecord& value,
        unsigned int version) const {
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("order_id", value.m_order_id);
      shuttle.shuttle("rule_id", value.m_rule_id);
      shuttle.shuttle("schema_name", value.m_schema_name);
      shuttle.shuttle("reason", value.m_reason);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
