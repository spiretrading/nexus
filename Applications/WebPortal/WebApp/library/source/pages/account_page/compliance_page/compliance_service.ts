import { ComplianceModel } from './compliance_model';

/** Interface for the services needed by the CompliancePage. */
export abstract class ComplianceService {

  /** Loads this model. */
  public abstract load(): Promise<ComplianceModel>;

  /**
   * Submits a request to commit all changes made to a ComplianceModel.
   * @param model The ComplianceModel to commit.
   * @return The updated ComplianceModel after the changes have been committed.
   */
  public abstract submit(model: ComplianceModel): Promise<ComplianceModel>;
}
