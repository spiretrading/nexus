import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Interface for a model representing an account's risk settings. */
export abstract class RiskModel {

  /** Returns the account represented. */
  public abstract get account(): Beam.DirectoryEntry;

  /** Returns the account's RiskParameters. */
  public abstract get riskParameters(): Nexus.RiskParameters;

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Submits a request to update the RiskParameters.
   * @param comment - The comment to include in the request.
   * @param riskParameters - The RiskParameters to request.
   */
  public abstract submit(comment: string,
    riskParameters: Nexus.RiskParameters): Promise<void>;
}
