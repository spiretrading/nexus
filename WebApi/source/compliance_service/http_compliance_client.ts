import * as Beam from 'beam';
import { ComplianceClient } from './compliance_client';
import { ComplianceRuleEntry } from './compliance_rule_entry';
import { ComplianceRuleSchema } from './compliance_rule_schema';
import { ComplianceRuleViolationRecord } from './compliance_rule_violation_record';

/** Implements the ComplianceClient using HTTP requests. */
export class HttpComplianceClient extends ComplianceClient {
  public async load(directoryEntry: Beam.DirectoryEntry):
      Promise<ComplianceRuleEntry[]> {
    const response = await Beam.post(
      '/api/compliance_service/load_directory_entry_compliance_rule_entry',
      {
        directory_entry: directoryEntry.toJson()
      });
    return Beam.arrayFromJson(ComplianceRuleEntry, response);
  }

  public async add(directoryEntry: Beam.DirectoryEntry,
      state: ComplianceRuleEntry.State, schema: ComplianceRuleSchema):
      Promise<number> {
    const response = await Beam.post(
      '/api/compliance_service/add_compliance_rule_entry',
      {
        directory_entry: directoryEntry.toJson(),
        state: Beam.toJson(state),
        schema: schema.toJson()
      });
    return response;
  }

  public async update(entry: ComplianceRuleEntry): Promise<void> {
    const response = await Beam.post(
      '/api/compliance_service/update_compliance_rule_entry',
      {
        rule_entry: entry.toJson()
      });
  }

  public async delete(id: number): Promise<void> {
    const response = await Beam.post(
      '/api/compliance_service/delete_compliance_rule_entry',
      {
        id: id
      });
  }

  public async report(violationRecord: ComplianceRuleViolationRecord):
      Promise<void> {
  }

  public async open(): Promise<void> {
    return;
  }

  public async close(): Promise<void> {
    return;
  }
}
