import * as Nexus from 'nexus';
import { ComplianceModel } from './compliance_model';

/** Interface for the services needed by the CompliancePage. */
export abstract class ComplianceService {

  /** Loads this model. */
  public abstract load(): Promise<ComplianceModel>;

  /**
   * Submits a request to update the applicable rules.
   * @param entries The list of updated entries.
   */
  public abstract submit(entries: Nexus.ComplianceRuleEntry[]): Promise<void>;
}
