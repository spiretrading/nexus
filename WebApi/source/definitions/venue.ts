import * as Beam from 'beam';
import { CountryCode } from './country';
import { Currency } from './currency';

/** Stores a venue's unique identifer. */
export class Venue {

  /** Represents no venue. */
  public static readonly NONE = new Venue('');

  /** Parses a Venue from JSON. */
  public static fromJson(value: any): Venue {
    return new Venue(value);
  }

  /** Constructs a Venue from its market identifier code. */
  constructor(code: string) {
    this.code = code;
  }

  /** Tests if two Venues are equal. */
  public equals(other: Venue): boolean {
    return other && this.code === other.code;
  }

  /** Returns the market identifier code. */
  public toString(): string {
    return this.code;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return this.code;
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return Beam.hashString(this.code);
  }

  private code: string;
}

/** Stores a database of venues. */
export class VenueDatabase {

  /** Parses a database from a JSON object. */
  public static fromJson(value: any): VenueDatabase {
    const database = new VenueDatabase();
    database._entries = Beam.arrayFromJson(VenueDatabase.Entry, value.entries);
    return database;
  }

  /** Constructs an empty database. */
  constructor() {
    this._entries = [];
  }

  /**
   * Returns an entry from its code.
   * @param venue - The Venue to lookup.
   * @return The Entry with the specified code.
   */
  public fromVenue(venue: Venue): VenueDatabase.Entry {
    for(const entry of this._entries) {
      if(entry.venue.equals(venue)) {
        return entry;
      }
    }
    return VenueDatabase.Entry.NONE;
  }

  /**
   * Returns an entry from its display name.
   * @param displayName The venue's display name.
   * @return The Entry with the specified display name.
   */
  public fromDisplayName(displayName: string): VenueDatabase.Entry {
    for(const entry of this._entries) {
      if(entry.displayName === displayName) {
        return entry;
      }
    }
    return VenueDatabase.Entry.NONE;
  }

  /**
   * Adds an entry.
   * @param entry - The entry to add.
   */
  public add(entry: VenueDatabase.Entry): void {
    if(this.fromVenue(entry.venue).equals(VenueDatabase.Entry.NONE)) {
      this._entries.push(entry);
    }
  }

  /**
   * Removes an entry.
   * @param venue - The Venue whose entry is to be removed.
   */
  public delete(venue: Venue): void {
    for(let i = 0; i < this._entries.length; ++i) {
      if(this._entries[i].venue.equals(venue)) {
        this._entries.splice(i, 1);
        break;
      }
    }
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return Beam.arrayToJson(this._entries);
  }

  [Symbol.iterator]() {
    return this._entries[Symbol.iterator]();
  }

  private _entries: VenueDatabase.Entry[];
}

export module VenueDatabase {

  /** Stores a single venue database entry. */
  export class Entry {

    /** Represents an invalid entry. */
    public static readonly NONE = new Entry(Venue.NONE, CountryCode.NONE, '',
      'UTC', Currency.NONE, 'None', '');

    /** Constructs an entry from a JSON object. */
    public static fromJson(value: any) {
      return new Entry(Venue.fromJson(value.venue),
        CountryCode.fromJson(value.country_code), value.market_center,
        value.time_zone, Currency.fromJson(value.currency), value.description,
        value.display_name);
    }

   /**
    * Parses a VenueEntry from a string.
    * @param source The string to parse.
    * @param database The VenueDatabase containing the available VenueEntry.
    * @return The Venue represented by the source.
    */
    public static parse(source: string, database: VenueDatabase): Entry {
      const entry = database.fromDisplayName(source);
      if(entry.venue.equals(Venue.NONE)) {
        return database.fromVenue(new Venue(source));
      }
      return entry;
    }

    /**
     * Constructs an entry.
     * @param venue - The venue.
     * @param countryCode - The venue's country code.
     * @param marketCenter - The default market center used for trades.
     * @param timeZone - The venue's time zone.
     * @param currency - The venue's default currency.
     * @param description - The institution's description.
     * @param displayName - The common display name.
     */
    constructor(venue: Venue, countryCode: CountryCode, marketCenter: string,
        timeZone: string, currency: Currency, description: string,
        displayName: string) {
      this._venue = venue;
      this._countryCode = countryCode;
      this._marketCenter = marketCenter;
      this._timeZone = timeZone;
      this._currency = currency;
      this._description = description;
      this._displayName = displayName;
    }

    /** Tests two entries for equality. */
    public equals(other: Entry): boolean {
      return other && this._venue.equals(other._venue) &&
        this._countryCode.equals(other._countryCode) &&
        this._marketCenter === other._marketCenter &&
        this._timeZone === other._timeZone &&
        this._currency.equals(other._currency) &&
        this._description === other._description &&
        this._displayName === other._displayName;
    }

    /** Returns the venue. */
    public get venue(): Venue {
      return this._venue;
    }

    /** Returns the venue's country code. */
    public get countryCode(): CountryCode {
      return this._countryCode;
    }

    /** Returns the default market center used for trades */
    public get marketCenter(): string {
      return this._marketCenter;
    }

    /** Returns the venue's time zone. */
    public get timeZone(): string {
      return this._timeZone;
    }

    /** Returns the venue's default currency. */
    public get currency(): Currency {
      return this._currency;
    }

    /** Returns the institution's description. */
    public get description(): string {
      return this._description;
    }

    /** Returns the venue's display name. */
    public get displayName(): string {
      return this._displayName;
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        venue: this._venue.toJson(),
        market_center: this._marketCenter,
        country_code: this._countryCode.toJson(),
        time_zone: this._timeZone,
        currency: this._currency.toJson(),
        description: this._description,
        display_name: this._displayName
      };
    }

    private _venue: Venue;
    private _countryCode: CountryCode;
    private _marketCenter: string;
    private _timeZone: string;
    private _currency: Currency;
    private _description: string;
    private _displayName: string;
  }
}
