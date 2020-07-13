#ifndef NEXUS_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_COMPLIANCE_RULE_DATA_STORE_HPP
#include <vector>
#include <Beam/Utilities/Concept.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {

  /** Models a data store for all compliance rules. */
  struct ComplianceRuleDataStore : Beam::Concept<ComplianceRuleDataStore> {

    /** Returns all ComplianceRuleEntries. */
    std::vector<ComplianceRuleEntry> LoadAllComplianceRuleEntries();

    /** Loads the next id to use for a ComplianceRuleEntry. */
    ComplianceRuleId LoadNextComplianceRuleEntryId();

    /**
     * Loads a ComplianceRuleEntry from its id.
     * @param id The id of the ComplianceRuleEntry to load.
     * @return The ComplianceRuleEntry with the specified <i>id</i>.
     */
    boost::optional<ComplianceRuleEntry> LoadComplianceRuleEntry(
      ComplianceRuleId id);

    /**
     * Loads all ComplianceRuleEntries for a specified DirectoryEntry.
     * @param directoryEntry The DirectoryEntry to search for.
     * @return The list of ComplianceRuleEntries assigned to the specified
     *         <i>directoryEntry</i>.
     */
    std::vector<ComplianceRuleEntry> LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

    /**
     * Stores a ComplianceRuleEntry.
     * @param entry The ComplianceRuleEntry to store.
     */
    void Store(const ComplianceRuleEntry& entry);

    /**
     * Deletes a ComplianceRuleEntry.
     * @param id The id of the ComplianceRuleEntry to delete.
     */
    void Delete(ComplianceRuleId id);

    /**
     * Stores a ComplianceRuleViolationRecord.
     * @param violationRecord The violation to store.
     */
    void Store(const ComplianceRuleViolationRecord& violationRecord);

    void Open();

    void Close();
  };
}

#endif
