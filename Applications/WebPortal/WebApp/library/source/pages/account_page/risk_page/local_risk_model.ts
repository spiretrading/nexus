import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { RiskModel } from "./risk_model";

/** Implements a RiskModel in memory. */
export class LocalRiskModel extends RiskModel {

  /** Constructs a model.
   * @param account - The account to represent.
   * @param riskParameters - The RiskParameters to associate with the account.
   */
  constructor(account: Beam.DirectoryEntry,
      riskParameters: Nexus.RiskParameters) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._riskParameters = riskParameters.clone();
  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public get account(): Beam.DirectoryEntry {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._account;
  }

  public get riskParameters(): Nexus.RiskParameters {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._riskParameters.clone();
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async submit(comment: string,
      riskParameters: Nexus.RiskParameters): Promise<void> {
    return;
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _riskParameters: Nexus.RiskParameters;
}
