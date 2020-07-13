#ifndef NEXUS_COMPLIANCE_SERVICES_HPP
#define NEXUS_COMPLIANCE_SERVICES_HPP
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {
  BEAM_DEFINE_SERVICES(ComplianceServices,

    /**
     * Loads all of a DirectoryEntry's ComplianceRuleEntries.
     * @param directory_entry The DirectoryEntry to query.
     * @return The list of ComplianceRuleEntries assigned to the
     *         <i>directory_entry</i>.
     */
    (LoadDirectoryEntryComplianceRuleEntryService,
      "Nexus.Compliance.LoadDirectoryEntryComplianceRuleEntryService",
      std::vector<ComplianceRuleEntry>, Beam::ServiceLocator::DirectoryEntry,
      directory_entry),

    /**
     * Monitors changes to a DirectoryEntry's ComplianceRuleEntries.
     * @param directory_entry The DirectoryEntry to monitor.
     */
    (MonitorComplianceRuleEntryService,
      "Nexus.Compliance.MonitorComplianceRuleEntryService", void,
      Beam::ServiceLocator::DirectoryEntry, directory_entry),

    /**
     * Adds a ComplianceRuleEntry.
     * @param directory_entry The DirectoryEntry to assign the rule to.
     * @param state The initial state of the entry.
     * @schema The ComplianceRuleSchema specifying the rule to add.
     * @return The id of the new entry.
     */
    (AddComplianceRuleEntryService,
      "Nexus.Compliance.AddComplianceRuleEntryService", ComplianceRuleId,
      Beam::ServiceLocator::DirectoryEntry, directory_entry,
      ComplianceRuleEntry::State, state, ComplianceRuleSchema, schema),

    /**
     * Updates a ComplianceRuleEntry.
     * @param entry The updated ComplianceRuleEntry.
     */
    (UpdateComplianceRuleEntryService,
      "Nexus.Compliance.UpdateComplianceRuleEntryService", void,
      ComplianceRuleEntry, entry),

    /**
     * Deletes a ComplianceRuleEntry.
     * @param id The id of the ComplianceRuleEntry to delete.
     */
    (DeleteComplianceRuleEntryService,
      "Nexus.Compliance.DeleteComplianceRuleEntryService", void,
      ComplianceRuleId, id));

  BEAM_DEFINE_MESSAGES(ComplianceMessages,

    /**
     * Signals a change to a ComplianceRuleEntry.
     * @param compliance_rule_entry The new or updated ComplianceRuleEntry.
     */
    (ComplianceRuleEntryMessage, "Nexus.Compliance.ComplianceRuleEntryMessage",
      ComplianceRuleEntry, compliance_rule_entry),

    /**
     * Reports a compliance violation.
     * @param violation_record The violation to report.
     */
    (ReportComplianceRuleViolationMessage,
      "Nexus.Compliance.ReportComplianceRuleViolationMessage",
      ComplianceRuleViolationRecord, violation_record));
}

#endif
