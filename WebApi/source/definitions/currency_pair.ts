import { Currency } from './currency';

/** Represents a pair of currencies. */
export class CurrencyPair {

  /** Constructs a CurrencyPair between two NONE currencies. */
  public static readonly NONE = new CurrencyPair(Currency.NONE, Currency.NONE);

  /** Parses a CurrencyPair from a JSON object. */
  public static fromJson(value: any): CurrencyPair {
    return new CurrencyPair(
      Currency.fromJson(value.base), Currency.fromJson(value.counter));
  }

  /**
   * Constructs a CurrencyPair.
   * @param base - The base currency.
   * @param counter - The counter currency.
   */
  constructor(base: Currency, counter: Currency) {
    this._base = base;
    this._counter = counter;
  }

  /** Returns the base currency. */
  public get base(): Currency {
    return this._base;
  }

  /** Returns the counter currency. */
  public get counter(): Currency {
    return this._counter;
  }

  /** Tests if two CurrencyPairs are equal. */
  public equals(other: CurrencyPair): boolean {
    return other && this._base.equals(other._base) &&
      this._counter.equals(other._counter);
  }

  /** Returns the CurrencyPair with the base and counter inverted. */
  public invert(): CurrencyPair {
    return new CurrencyPair(this._counter, this._base);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      base: this._base.toJson(),
      counter: this._counter.toJson()
    };
  }

  /** Returns a hash of this value. */
  public hash(): number {
    return 31 * this._base.hash() + this._counter.hash();
  }

  private _base: Currency;
  private _counter: Currency;
}
