import * as Nexus from 'nexus';
import { ComplianceModel } from './compliance_model';
import { ComplianceService } from './compliance_service';

/** Implements an in-memory ComplianceService. */
export class LocalComplianceService extends ComplianceService {

  /**
   * Constructs a LocalComplianceService.
   * @param model The model to return on load.
   */
  constructor(model: ComplianceModel) {
    super();
    this.model = model.clone();
  }

  public async load(): Promise<ComplianceModel> {
    return this.model.clone();
  }

  public submit(entries: Nexus.ComplianceRuleEntry[]): Promise<void> {
    return;
  }

  private model: ComplianceModel;
}
