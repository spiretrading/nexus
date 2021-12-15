import { RiskParameters } from '..';

/** Represents a request to modify risk parameters. */
export class RiskModification {

  /** Constructs a RiskModification from a JSON object. */
  public static fromJson(value: any): RiskModification {
    return new RiskModification(RiskParameters.fromJson(value));
  }

  /**
   * Constructs a RiskModification.
   * @param parameters - The risk parameters being requested.
   */
  constructor(parameters: RiskParameters) {
    this._parameters = parameters.clone();
  }

  /** Returns the risk parameters. */
  public get parameters() {
    return this._parameters;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      parameters: this._parameters.toJson()
    };
  }

  private _parameters: RiskParameters;
}
