import * as Beam from 'beam';
import { MarketCode } from '..';

/** Stores an index into a market data entitlement. */
export class EntitlementKey {

  /** Parses an EntitlementKey from JSON. */
  public static fromJson(value: any): EntitlementKey {
    return new EntitlementKey(MarketCode.fromJson(value.market),
      MarketCode.fromJson(value.source));
  }

  /**
   * Constructs an EntitlementKey.
   * @param market - The market the security belongs to.
   * @param source - The market disseminating the data, the default value is
   *        the market.
   */
  constructor(market: MarketCode, source?: MarketCode) {
    this._market = market;
    if(source) {
      this._source = source;
    } else {
      this._source = market;
    }
  }

  /** Returns the market the security belongs to. */
  public get market(): MarketCode {
    return this._market;
  }

  /** Returns the market disseminating the data. */
  public get source(): MarketCode {
    return this._source;
  }

  /** Tests if two keys are equal. */
  public equals(other: EntitlementKey): boolean {
    return other && this._market.equals(other._market) &&
      this._source.equals(other._source);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      market: this._market.toJson(),
      source: this._source.toJson()
    };
  }

  /** Returns a hash of this object. */
  public hash(): number {
    let seed = 0;
    seed = Beam.hashCombine(seed, this._market.hash());
    seed = Beam.hashCombine(seed, this._source.hash());
    return seed;
  }

  private _market: MarketCode;
  private _source: MarketCode;
}
