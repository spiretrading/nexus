import * as Beam from 'beam';

/** Identifies a country by its ISO 3166-1 numeric code. */
export class CountryCode {

  /** Represents no country. */
  public static readonly NONE = new CountryCode(~0);

  /** Parses a CountryCode from JSON. */
  public static fromJson(value: any): CountryCode {
    return new CountryCode(value);
  }

  /** Constructs a CountryCode from its ISO 3166-1 numeric code. */
  constructor(code: number) {
    this._code = code;
  }

  /** Returns the ISO 3166-1 numeric code. */
  public get code(): number {
    return this._code;
  }

  /** Tests if two CountryCodes represent the same country. */
  public equals(other: CountryCode): boolean {
    return other && this._code === other._code;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return this._code;
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return this._code;
  }

  private _code: number;
}

/** Stores a database of countries. */
export class CountryDatabase {

  /** Parses a database from a JSON object. */
  public static fromJson(value: any): CountryDatabase {
    const database = new CountryDatabase();
    database._entries = Beam.arrayFromJson(CountryDatabase.Entry,
      value.entries);
    return database;
  }

  /** Constructs an empty database. */
  constructor() {
    this._entries = [];
  }

  /**
   * Returns an entry from a CountryCode.
   * @param code - The CountryCode to lookup.
   * @return The entry with the specified code.
   */
  public fromCode(code: CountryCode): CountryDatabase.Entry {
    for(const entry of this._entries) {
      if(entry.code.equals(code)) {
        return entry;
      }
    }
    return CountryDatabase.Entry.NONE;
  }

  /**
   * Adds an entry.
   * @param entry - The entry to add.
   */
  public add(entry: CountryDatabase.Entry): void {
    if(this.fromCode(entry.code).code.equals(CountryCode.NONE)) {
      this._entries.push(entry);
    }
  }

  /**
   * Removes an entry.
   * @param code - The country code whose entry is to be removed.
   */
  public delete(code: CountryCode): void {
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

  private _entries: CountryDatabase.Entry[];
}

export module CountryDatabase {

  /** Stores a single country database entry. */
  export class Entry {

    /** Represents an invalid entry. */
    public static readonly NONE = new Entry(CountryCode.NONE, 'None', '??',
      '???');

    /** Constructs an entry from a JSON object. */
    public static fromJson(value: any) {
      return new Entry(CountryCode.fromJson(value.code), value.name,
        value.two_letter_code, value.three_letter_code);
    }

    /**
     * Constructs an entry.
     * @param code - The country code.
     * @param name - The name of the country.
     * @param twoLetterCode - The country's two letter code.
     * @param threeLetterCode - The country's three letter code.
     */
    constructor(code: CountryCode, name: string, twoLetterCode: string,
        threeLetterCode: string) {
      this._code = code;
      this._name = name;
      this._twoLetterCode = twoLetterCode;
      this._threeLetterCode = threeLetterCode;
    }

    /** Returns the country code. */
    public get code(): CountryCode {
      return this._code;
    }

    /** Returns the country's name.*/
    public get name(): string {
      return this._name;
    }

    /** Returns the country's two letter code.*/
    public get twoLetterCode(): string {
      return this._twoLetterCode;
    }

    /** Returns the country's three letter code.*/
    public get threeLetterCode(): string {
      return this._threeLetterCode;
    }

    /** Tests two entries for equality. */
    public equals(other: Entry): boolean {
      return other && this._code.equals(other._code) &&
        this._name === other._name &&
        this._twoLetterCode === other._twoLetterCode &&
        this._threeLetterCode === other._threeLetterCode;
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        code: this._code.toJson(),
        name: this._name,
        two_letter_code: this._twoLetterCode,
        three_letter_code: this._threeLetterCode
      };
    }

    private _code: CountryCode;
    private _name: string;
    private _twoLetterCode: string;
    private _threeLetterCode: string;
  }
}
