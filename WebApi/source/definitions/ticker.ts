import * as Beam from 'beam';
import { defaultVenueDatabase } from './default_venue_database';
import { Venue, VenueDatabase } from './venue';

/** Identifies a financial ticker. */
export class Ticker {

  /** Represents no Ticker. */
  public static readonly NONE = new Ticker('', Venue.NONE);

  /** Makes a value from a JSON object. */
  public static fromJson(value: any): Ticker {
    return new Ticker(value.symbol, Venue.fromJson(value.venue));
  }

  /** Parses a value from a string. */
  public static parse(source: string, venueDatabase?: VenueDatabase): Ticker {
    const seperator = source.lastIndexOf('.');
    if(seperator === -1) {
      return Ticker.NONE;
    }
    const symbol = source.substring(0, seperator);
    const venueSource = new Venue(source.substring(seperator + 1));
    const database = venueDatabase || defaultVenueDatabase;
    let venue = database.fromDisplayName(venueSource.toString());
    if(venue.venue.equals(Venue.NONE)) {
      venue = database.fromVenue(venueSource);
      if(venue.venue.equals(Venue.NONE)) {
        return Ticker.NONE;
      }
    }
    return new Ticker(symbol, venue.venue);
  }

  /**
   * Constructs a Ticker.
   * @param symbol - The ticker's symbol.
   * @param venue - The ticker's venue.
   */
  public constructor(symbol: string, venue: Venue) {
    this._symbol = symbol;
    this._venue = venue;
  }

  /** Returns the ticker symbol. */
  public get symbol(): string {
    return this._symbol;
  }

  /** Returns the venue. */
  public get venue(): Venue {
    return this._venue;
  }

  /** Tests if two Tickers are equal. */
  public equals(other: Ticker): boolean {
    return this.symbol == other.symbol && this.venue.equals(other.venue);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      symbol: this._symbol,
      venue: this._venue.toJson()
    };
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return Beam.hashCombine(Beam.hash(this.symbol), Beam.hash(this.venue));
  }

  public toString(venueDatabase?: VenueDatabase): string {
    if(this._venue.equals(Venue.NONE) || this._symbol === '') {
      return this._symbol;
    }
    const database = venueDatabase || defaultVenueDatabase;
    const venue = database.fromVenue(this._venue);
    if(venue.venue.equals(Venue.NONE)) {
      return `${this._symbol}.${this._venue.toString()}`;
    }
    return `${this._symbol}.${venue.displayName}`;
  }

  private _symbol: string;
  private _venue: Venue;
}
