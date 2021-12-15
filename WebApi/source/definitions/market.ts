import * as Beam from 'beam';
import { CountryCode } from './country';
import { Currency } from './currency';

/** Stores a market's unique identifer. */
export class MarketCode {

  /** Represents no market. */
  public static readonly NONE = new MarketCode('');

  /** Parses a MarketCode from JSON. */
  public static fromJson(value: any): MarketCode {
    return new MarketCode(value);
  }

  /** Constructs a MarketCode from its identifier. */
  constructor(identifier: string) {
    this.identifier = identifier;
  }

  /** Tests if two MarketCodes represent the same market. */
  public equals(other: MarketCode): boolean {
    return other && this.identifier === other.identifier;
  }

  /** Returns the market's identifier. */
  public toString(): string {
    return this.identifier;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return this.identifier;
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return Beam.hashString(this.identifier);
  }

  private identifier: string;
}

/** Stores a database of markets. */
export class MarketDatabase {

  /** Parses a database from a JSON object. */
  public static fromJson(value: any): MarketDatabase {
    const database = new MarketDatabase();
    database._entries = Beam.arrayFromJson(MarketDatabase.Entry, value.entries);
    return database;
  }

  /** Constructs an empty database. */
  constructor() {
    this._entries = [];
  }

  /**
   * Returns an entry from its code.
   * @param code - The MarketCode to lookup.
   * @return The Entry with the specified code.
   */
  public fromCode(code: MarketCode): MarketDatabase.Entry {
    for(const entry of this._entries) {
      if(entry.code.equals(code)) {
        return entry;
      }
    }
    return MarketDatabase.Entry.NONE;
  }

  /**
   * Returns an entry from its display name.
   * @param displayName The market's display name.
   * @return The Entry with the specified display name.
   */
  public fromDisplayName(displayName: string): MarketDatabase.Entry {
    for(const entry of this._entries) {
      if(entry.displayName === displayName) {
        return entry;
      }
    }
    return MarketDatabase.Entry.NONE;
  }

  /**
   * Adds an entry.
   * @param entry - The entry to add.
   */
  public add(entry: MarketDatabase.Entry): void {
    if(this.fromCode(entry.code).equals(MarketDatabase.Entry.NONE)) {
      this._entries.push(entry);
    }
  }

  /**
   * Removes an entry.
   * @param code - The MarketCode whose entry is to be removed.
   */
  public delete(code: MarketCode): void {
    for(let i = 0; i < this._entries.length; ++i) {
      if(this._entries[i].code.equals(code)) {
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

  private _entries: MarketDatabase.Entry[];
}

export module MarketDatabase {

  /** Stores a single market database entry. */
  export class Entry {

    /** Represents an invalid entry. */
    public static readonly NONE = new Entry(MarketCode.NONE, CountryCode.NONE,
      'UTC', Currency.NONE, 0, 'None', '');

    /** Constructs an entry from a JSON object. */
    public static fromJson(value: any) {
      return new Entry(MarketCode.fromJson(value.code),
        CountryCode.fromJson(value.country_code), value.time_zone,
        Currency.fromJson(value.currency), value.board_lot, value.description,
        value.display_name);
    }

   /**
    * Parses a MarketEntry from a string.
    * @param source The string to parse.
    * @param database The MarketDatabase containing the available MarketEntry.
    * @return The MarketCode represented by the source.
    */
    public static parse(source: string, database: MarketDatabase): Entry {
      const entry = database.fromDisplayName(source);
      if(entry.code.equals(MarketCode.NONE)) {
        return database.fromCode(new MarketCode(source));
      }
      return entry;
    }

    /**
     * Constructs an entry.
     * @param code - The market identifier code.
     * @param countryCode - The market's country code.
     * @param timeZone - The market's time zone.
     * @param currency - The market's default currency.
     * @param boardLot - The default board lot size.
     * @param description - The institution's description.
     * @param displayName - The common display name.
     */
    constructor(code: MarketCode, countryCode: CountryCode, timeZone: string,
        currency: Currency, boardLot: number, description: string,
        displayName: string) {
      this._code = code;
      this._countryCode = countryCode;
      this._timeZone = timeZone;
      this._currency = currency;
      this._boardLot = boardLot;
      this._description = description;
      this._displayName = displayName;
    }

    /** Tests two entries for equality. */
    public equals(other: Entry): boolean {
      return other && this._code.equals(other._code) &&
        this._countryCode.equals(other._countryCode) &&
        this._timeZone === other._timeZone &&
        this._currency.equals(other._currency) &&
        this._boardLot === other._boardLot &&
        this._description === other._description &&
        this._displayName === other._displayName;
    }

    /** Returns the market identifier code. */
    public get code(): MarketCode {
      return this._code;
    }

    /** Returns the market's country code. */
    public get countryCode(): CountryCode {
      return this._countryCode;
    }

    /** Returns the market's time zone. */
    public get timeZone(): string {
      return this._timeZone;
    }

    /** Returns the market's default currency. */
    public get currency(): Currency {
      return this._currency;
    }

    /** Returns the market's default board lot. */
    public get boardLot(): number {
      return this._boardLot;
    }

    /** Returns the institution's description. */
    public get description(): string {
      return this._description;
    }

    /** Returns the market's display name. */
    public get displayName(): string {
      return this._displayName;
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        code: this._code.toJson(),
        country_code: this._countryCode.toJson(),
        time_zone: this._timeZone,
        currency: this._currency.toJson(),
        board_lot: this._boardLot,
        description: this._description,
        display_name: this._displayName
      };
    }

    private _code: MarketCode;
    private _countryCode: CountryCode;
    private _timeZone: string;
    private _currency: Currency;
    private _boardLot: number;
    private _description: string;
    private _displayName: string;
  }
}
