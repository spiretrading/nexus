import Fraction from 'fraction.js';
import { CurrencyPair } from './currency_pair';
import { Money } from './money';

/** Represents a CurrencyPair's exchange rate. */
export class ExchangeRate {

  /**
   * Constructs an ExchangeRate between two NONE currencies with a rate of 1.
   */
  public static readonly NONE = new ExchangeRate(CurrencyPair.NONE,
    new Fraction(1));

  /** Parses an ExchangeRate from a JSON object. */
  public static fromJson(value: any): ExchangeRate {
    return new ExchangeRate(CurrencyPair.fromJson(value.pair),
      new Fraction(value.rate.numerator, value.rate.denominator));
  }

  /**
   * Constructs an ExchangeRate.
   * @param pair - The CurrencyPair this rate represents.
   * @param rate - The conversion rate from the base to the counter currency.
   */
  constructor(pair: CurrencyPair, rate: Fraction) {
    this._pair = pair;
    this._rate = rate;
  }

  /** Returns the CurrencyPair this rate represents. */
  public get pair(): CurrencyPair {
    return this._pair;
  }

  /** Returns the conversion rate. */
  public get rate(): Fraction {
    return this._rate;
  }

  /** Tests if two ExchangeRates are equal. */
  public equals(other: ExchangeRate): boolean {
    return other && this._pair.equals(other._pair) &&
      this._rate.equals(other._rate);
  }

  /** Returns the ExchangeRate with the base and counter inverted. */
  public invert(): ExchangeRate {
    return new ExchangeRate(this._pair.invert(), this._rate.inverse());
  }

  /** Converts a Money value according to this ExchangeRate. */
  public convert(value: Money): Money {
    return value.multiply(this._rate.n).divide(this._rate.d);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      pair: this._pair.toJson(),
      rate: {
        numerator: this._rate.n,
        denominator: this._rate.d
      }
    };
  }

  private _pair: CurrencyPair;
  private _rate: Fraction;
}
