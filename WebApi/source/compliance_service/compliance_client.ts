import * as Beam from 'beam';
import { ComplianceRuleEntry } from './compliance_rule_entry';
import { ComplianceRuleSchema } from './compliance_rule_schema';
import { ComplianceRuleViolationRecord } from './compliance_rule_violation_record';

/** Client used to access compliance services. */
export abstract class ComplianceClient {

  /**
   * Loads all ComplianceRuleEntries for a specified DirectoryEntry.
   * @param directoryEntry The DirectoryEntry to query.
   * @return The list of all ComplianceRuleEntries assigned to the
   *         directoryEntry.
   */
  public abstract load(directoryEntry: Beam.DirectoryEntry):
    Promise<ComplianceRuleEntry[]>;

  /**
   * Assigns a new compliance rule to a DirectoryEntry.
   * @param directoryEntry The DirectoryEntry to assign the rule to.
   * @param state The rule's initial State.
   * @param schema The ComplianceRuleSchema specifying the rule to add.
   * @return The id of the new entry.
   */
  public abstract add(directoryEntry: Beam.DirectoryEntry,
    state: ComplianceRuleEntry.State, schema: ComplianceRuleSchema):
    Promise<number>;

  /**
   * Updates an existing compliance rule.
   * @param entry The ComplianceRuleEntry to update.
   */
  public abstract update(entry: ComplianceRuleEntry): Promise<void>;

  /**
   * Deletes a ComplianceRuleEntry.
   * @param id The ComplianceRuleId to delete.
   */
  public abstract delete(id: number): Promise<void>;

  /**
   * Reports a compliance violation.
   * @param violationRecord The violation to report.
   */
  public abstract report(
    violationRecord: ComplianceRuleViolationRecord): Promise<void>;

  /**
   * Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}
