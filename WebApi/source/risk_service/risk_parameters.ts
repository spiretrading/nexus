import * as Beam from 'beam';
import { Currency, Money } from '..';
import { RiskState } from './risk_state';

/** Stores an account's risk parameters. */
export class RiskParameters {

  /** Represents an invalid value. */
  public static readonly INVALID = new RiskParameters(Currency.NONE,
    Money.ZERO, RiskState.NONE, Money.ZERO, Beam.Duration.ZERO);

  /** Parses RiskParameters from JSON. */
  public static fromJson(value: any): RiskParameters {
    return new RiskParameters(Currency.fromJson(value.currency),
      Money.fromJson(value.buying_power),
      RiskState.fromJson(value.allowed_state), Money.fromJson(value.net_loss),
      Beam.Duration.fromJson(value.transition_time));
  }

  /**
   * Constructs RiskParameters.
   * @param currency - The currency used for risk calculations.
   * @param buyingPower - The maximum amount of buying power.
   * @param allowedState - The default risk state.
   * @param netLoss - The maximum net loss before entering closed orders mode.
   * @param transitionTime - The amount of time allowed to transition from
   *        closed orders mode to disabled mode.
   */
  constructor(currency: Currency, buyingPower: Money, allowedState: RiskState,
      netLoss: Money, transitionTime: Beam.Duration) {
    this._currency = currency;
    this._buyingPower = buyingPower;
    this._allowedState = allowedState;
    this._netLoss = netLoss;
    this._transitionTime = transitionTime;
  }

  /** Makes a copy of this object. */
  public clone(): RiskParameters {
    return new RiskParameters(this._currency, this._buyingPower,
      this._allowedState, this._netLoss, this._transitionTime);
  }

  /** Returns the currency used for risk calculations. */
  public get currency(): Currency {
    return this._currency;
  }

  public set currency(value: Currency) {
    this._currency = value;
  }

  /** Returns the maximum amount of buying power. */
  public get buyingPower(): Money {
    return this._buyingPower;
  }

  public set buyingPower(value: Money) {
    this._buyingPower = value;
  }

  /** Returns the default risk state. */
  public get allowedState(): RiskState {
    return this._allowedState;
  }

  public set allowedState(value: RiskState) {
    this._allowedState = value;
  }

  /** Returns the maximum net loss before entering closed orders mode. */
  public get netLoss(): Money {
    return this._netLoss;
  }

  public set netLoss(value: Money) {
    this._netLoss = value;
  }

  /**
   * The amount of time allowed to transition from closed orders mode to
   * disabled mode.
   */
  public get transitionTime(): Beam.Duration {
    return this._transitionTime;
  }

  public set transitionTime(value: Beam.Duration) {
    this._transitionTime = value;
  }

  /** Tests if two risk parameters are equal. */
  public equals(other: RiskParameters): boolean {
    return other && this._currency.equals(other.currency) &&
      this._buyingPower.equals(other.buyingPower) && this._allowedState.equals(
      other.allowedState) && this._netLoss.equals(other.netLoss) &&
      this._transitionTime.equals(other.transitionTime);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      currency: this._currency.toJson(),
      buying_power: this._buyingPower.toJson(),
      allowed_state: this._allowedState.toJson(),
      net_loss: this._netLoss.toJson(),
      transition_time: this._transitionTime.toJson()
    };
  }

  private _currency: Currency;
  private _buyingPower: Money;
  private _allowedState: RiskState;
  private _netLoss: Money;
  private _transitionTime: Beam.Duration;
}
