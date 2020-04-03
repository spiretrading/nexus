import * as Beam from 'beam';

/** Stores the risk monitoring state of an account. */
export class RiskState {

  /** Represents a NONE state. */
  public static readonly NONE = new RiskState(-1);

  /** Builds a RiskState from a JSON object. */
  public static fromJson(value: any): RiskState {
    return new RiskState(value.type, Beam.DateTime.fromJson(value.expiry));
  }

  /** Constructs a RiskState.
   * @param type The state's type.
   * @param expiry When this state is expired to expire.
   */
  constructor(type: RiskState.Type,
      expiry: Beam.DateTime = Beam.DateTime.POS_INFIN) {
    this._type = type;
    this._expiry = expiry;
  }

  /** Returns the state's type. */
  public get type(): RiskState.Type {
    return this._type;
  }

  /** Returns the expiry. */
  public get expiry(): Beam.DateTime {
    return this._expiry;
  }

  /** Tests two states for equality. */
  public equals(other: RiskState): boolean {
    return this._type === other._type && this._expiry.equals(other._expiry);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      type: this._type,
      expiry: this._expiry.toJson()
    };
  }

  private _type: RiskState.Type;
  private _expiry: Beam.DateTime;
}

export module RiskState {

  /** The state that an account can be in for the purpose of risk monitoring. */
  export enum Type {

    /** Invalid state. */
    NONE = -1,

    /** The account is allowed to submit orders. */
    ACTIVE,

    /** The account may only submit orders to close a position. */
    CLOSED_ORDERS,

    /** The account may no longer submit orders. */
    DISABLED
  }
}
