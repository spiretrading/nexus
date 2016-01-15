#ifndef NEXUS_COMPLIANCESERVICES_HPP
#define NEXUS_COMPLIANCESERVICES_HPP
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {
namespace Compliance {
  BEAM_DEFINE_SERVICES(ComplianceServices,

    /*! \interface Nexus::Compliance::LoadDirectoryEntryComplianceRuleEntryService
        \brief Loads all of a DirectoryEntry's ComplianceRuleEntries.
        \param directory_entry <code>Beam::ServiceLocator::DirectoryEntry</code>
               The DirectoryEntry to query.
        \return <code>std::vector\<Nexus::Compliance::ComplianceRuleEntry\></code>
                The list of ComplianceRuleEntries assigned to the
                <i>directory_entry</i>.
    */
    //! \cond
    (LoadDirectoryEntryComplianceRuleEntryService,
      "Nexus.Compliance.LoadDirectoryEntryComplianceRuleEntryService",
      std::vector<ComplianceRuleEntry>, Beam::ServiceLocator::DirectoryEntry,
      directory_entry),
    //! \endcond

    /*! \interface Nexus::Compliance::MonitorComplianceRuleEntryService
        \brief Monitors changes to a DirectoryEntry's ComplianceRuleEntries.
        \param directory_entry <code>Beam::ServiceLocator::DirectoryEntry</code>
               The DirectoryEntry to monitor.
    */
    //! \cond
    (MonitorComplianceRuleEntryService,
      "Nexus.Compliance.MonitorComplianceRuleEntryService", void,
      Beam::ServiceLocator::DirectoryEntry, directory_entry),
    //! \endcond

    /*! \interface Nexus::Compliance::AddComplianceRuleEntryService
        \brief Adds a ComplianceRuleEntry.
        \param directory_entry <code>Beam::ServiceLocator::DirectoryEntry</code>
               The DirectoryEntry to assign the rule to.
        \param state <code>Nexus::Compliance::ComplianceRuleEntry::State</code>
               The initial state of the entry.
        \param schema <code>Nexus::Compliance::ComplianceRuleEntry</code> The
               ComplianceRuleSchema specifying the rule to add.
        \return <code>Nexus::Compliance::ComplianceRuleId</code> The id of the
                new entry.
    */
    //! \cond
    (AddComplianceRuleEntryService,
      "Nexus.Compliance.AddComplianceRuleEntryService", ComplianceRuleId,
      Beam::ServiceLocator::DirectoryEntry, directory_entry,
      ComplianceRuleEntry::State, state, ComplianceRuleSchema, schema),
    //! \endcond

    /*! \interface Nexus::Compliance::UpdateComplianceRuleEntryService
        \brief Updates a ComplianceRuleEntry.
        \param entry <code>Nexus::Compliance::ComplianceRuleEntry</code> The
               updated ComplianceRuleEntry.
    */
    //! \cond
    (UpdateComplianceRuleEntryService,
      "Nexus.Compliance.UpdateComplianceRuleEntryService", void,
      ComplianceRuleEntry, entry),
    //! \endcond

    /*! \interface Nexus::Compliance::DeleteComplianceRuleEntryService
        \brief Deletes a ComplianceRuleEntry.
        \param id <code>Nexus::Compliance::ComplianceRuleId</code> The id of the
               ComplianceRuleEntry to delete.
    */
    //! \cond
    (DeleteComplianceRuleEntryService,
      "Nexus.Compliance.DeleteComplianceRuleEntryService", void,
      ComplianceRuleId, id));
    //! \endcond

  BEAM_DEFINE_MESSAGES(ComplianceMessages,

    /*! \interface Nexus::Compliance::ComplianceRuleEntryMessage
        \brief Signals a change to a ComplianceRuleEntry.
        \param compliance_rule_entry <code>ComplianceRuleEntry</code>
               The new or updated ComplianceRuleEntry.
    */
    //! \cond
    (ComplianceRuleEntryMessage, "Nexus.Compliance.ComplianceRuleEntryMessage",
      ComplianceRuleEntry, compliance_rule_entry),
    //! \endcond

    /*! \interface Nexus::Compliance::ReportComplianceRuleViolationMessage
        \brief Reports a compliance violation.
        \param violation_record <code>Nexus::Compliance::ComplianceRuleViolationRecord</code>
               The violation to report.
    */
    //! \cond
    (ReportComplianceRuleViolationMessage,
      "Nexus.Compliance.ReportComplianceRuleViolationMessage",
      ComplianceRuleViolationRecord, violation_record));
    //! \endcond
}
}

#endif
