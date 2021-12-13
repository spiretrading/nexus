import * as Beam from 'beam';

/** Represents an ISO 4217 currency code. */
export class Currency {

  /** Represents no currency. */
  public static readonly NONE = new Currency(0);

  /** Parses a Currency from JSON. */
  public static fromJson(value: any): Currency {
    return new Currency(value);
  }

  /**
   * Constructs a Currency.
   * @param code - The ISO 4217 currency code.
   */
  constructor(code: number = 0) {
    this._code = code;
  }

  /** Returns the ISO 4217 currency code. */
  public get code(): number {
    return this._code;
  }

  /** Tests if two currencies are equal. */
  public equals(other: Currency): boolean {
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

/** Stores a database of currencies. */
export class CurrencyDatabase {

  /** Parses a database from a JSON object. */
  public static fromJson(value: any): CurrencyDatabase {
    const database = new CurrencyDatabase();
    database._entries = Beam.arrayFromJson(CurrencyDatabase.Entry,
      value.entries);
    return database;
  }

  /** Constructs an empty database. */
  constructor() {
    this._entries = [];
  }

  /**
   * Returns an entry from a currency.
   * @param currency - The currency to lookup.
   * @return The entry with the specified currency.
   */
  public fromCurrency(currency: Currency): CurrencyDatabase.Entry {
    for(let entry of this._entries) {
      if(entry.currency.equals(currency)) {
        return entry;
      }
    }
    return CurrencyDatabase.Entry.NONE;
  }

  /**
   * Returns an entry from a code.
   * @param code - The code to lookup.
   * @return The entry with the specified code.
   */
  public fromCode(code: string): CurrencyDatabase.Entry {
    for(let entry of this._entries) {
      if(entry.code === code) {
        return entry;
      }
    }
    return CurrencyDatabase.Entry.NONE;
  }

  /**
   * Adds an entry.
   * @param entry - The entry to add.
   */
  public add(entry: CurrencyDatabase.Entry): void {
    if(this.fromCode(entry.code).currency.equals(
        CurrencyDatabase.Entry.NONE.currency)) {
      this._entries.push(entry);
    }
  }

  /**
   * Removes an entry.
   * @param currency - The currency whose entry is to be removed.
   */
  public delete(currency: Currency): void {
    for(let i = 0; i < this._entries.length; ++i) {
      if(this._entries[i].currency.equals(currency)) {
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

  private _entries: CurrencyDatabase.Entry[];
}

export module CurrencyDatabase {

  /** Stores a single currency database entry. */
  export class Entry {

    /** Represents an invalid entry. */
    public static readonly NONE = new Entry(new Currency(), '?', '????');

    /** Constructs an entry from a JSON object. */
    public static fromJson(value: any) {
      return new Entry(Currency.fromJson(value.id), value.code, value.sign);
    }

    /**
     * Constructs an entry.
     * @param currency - The currency to represent.
     * @param code - The currency's code ie. (USD)
     * @param sign - The currency's sign ie. ($)
     */
    constructor(currency: Currency, code: string, sign: string) {
      this._currency = currency;
      this._code = code;
      this._sign = sign;
    }

    /** Returns the currency. */
    public get currency(): Currency {
      return this._currency;
    }

    /** Returns the currency code.*/
    public get code(): string {
      return this._code;
    }

    /** Returns the currency sign. */
    public get sign(): string {
      return this._sign;
    }

    /** Tests two entries for equality. */
    public equals(other: Entry): boolean {
      return other && this._currency.equals(other._currency) &&
        this._code === other._code && this._sign === other._sign;
    }

    /** Converts this object to JSON. */
    public toJson(): any {
      return {
        id: this._currency.code,
        code: this._code,
        sign: this._sign
      };
    }

    private _currency: Currency;
    private _code: string;
    private _sign: string;
  }
}
