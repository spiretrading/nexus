import * as Beam from 'beam';
import { Venue } from '..';

/** Stores an index into a market data entitlement. */
export class EntitlementKey {

  /** Parses an EntitlementKey from JSON. */
  public static fromJson(value: any): EntitlementKey {
    return new EntitlementKey(Venue.fromJson(value.venue),
      Venue.fromJson(value.source));
  }

  /**
   * Constructs an EntitlementKey.
   * @param venue - The venue the security belongs to.
   * @param source - The venue disseminating the data, the default value is
   *        the venue.
   */
  constructor(venue: Venue, source?: Venue) {
    this._venue = venue;
    if(source) {
      this._source = source;
    } else {
      this._source = venue;
    }
  }

  /** Returns the venue the security belongs to. */
  public get venue(): Venue {
    return this._venue;
  }

  /** Returns the venue disseminating the data. */
  public get source(): Venue {
    return this._source;
  }

  /** Tests if two keys are equal. */
  public equals(other: EntitlementKey): boolean {
    return other && this._venue.equals(other._venue) &&
      this._source.equals(other._source);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      venue: this._venue.toJson(),
      source: this._source.toJson()
    };
  }

  /** Returns a hash of this object. */
  public hash(): number {
    let seed = 0;
    seed = Beam.hashCombine(seed, this._venue.hash());
    seed = Beam.hashCombine(seed, this._source.hash());
    return seed;
  }

  private _venue: Venue;
  private _source: Venue;
}
