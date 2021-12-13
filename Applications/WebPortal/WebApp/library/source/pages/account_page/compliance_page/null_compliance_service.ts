import * as Nexus from 'nexus';
import { ComplianceModel } from './compliance_model';
import { ComplianceService } from './compliance_service';

/** Implements a no-op ComplianceService. */
export class NullComplianceService extends ComplianceService {
  public load(): Promise<ComplianceModel> {
    return;
  }

  public submit(entries: Nexus.ComplianceRuleEntry[]): Promise<void> {
    return;
  }
}
