import * as Beam from 'beam';
import Fraction from 'fraction.js';
import { Currency } from './currency';
import { CurrencyPair } from './currency_pair';
import { CurrencyPairNotFoundError } from './currency_pair_not_found_error';
import { ExchangeRate } from './exchange_rate';
import { Money } from './money';

/** Stores and retrieves exchange rates for currency conversions. */
export class ExchangeRateTable {

  /** Parses an exchange rate table from JSON. */
  public static fromJson(value: any): ExchangeRateTable {
    const table = new ExchangeRateTable();
    table._directRates = Beam.Map.fromJson<CurrencyPair, ExchangeRate>(
      CurrencyPair, ExchangeRate, value.direct_rates);
    return table;
  }

  /** Constructs an empty table. */
  constructor();

  /** Constructs a table from a list of exchange rates. */
  constructor(rates: ExchangeRate[]);
  constructor(rates?: ExchangeRate[]) {
    this._directRates = new Beam.Map<CurrencyPair, ExchangeRate>();
    this._derivedRates = new Beam.Map<CurrencyPair, ExchangeRate>();
    if(rates) {
      for(const rate of rates) {
        this.add(rate);
      }
    }
  }

  /**
   * Finds the exchange rate for a currency pair.
   * @param pair - The currency pair to look up.
   * @return The exchange rate, or null if no conversion exists.
   */
  public find(pair: CurrencyPair): ExchangeRate {
    const direct = this._directRates.get(pair);
    if(direct) {
      return direct;
    }
    const derived = this._derivedRates.get(pair);
    if(derived) {
      return derived;
    }
    if(pair.base.equals(pair.counter)) {
      return new ExchangeRate(pair, new Fraction(1));
    }
    const visited = new Set<number>([pair.base.code]);
    const queue: [Currency, Fraction][] = [[pair.base, new Fraction(1)]];
    for(let i = 0; i < queue.length; ++i) {
      const [currency, cumulative] = queue[i];
      if(currency.equals(pair.counter)) {
        const rate = new ExchangeRate(pair, cumulative);
        this._derivedRates.set(pair, rate);
        return rate;
      }
      for(const [pair, rate] of this._directRates) {
        let next: Currency;
        let factor: Fraction;
        if(pair.base.equals(currency)) {
          next = pair.counter;
          factor = rate.rate;
        } else if(pair.counter.equals(currency)) {
          next = pair.base;
          factor = rate.rate.inverse();
        } else {
          continue;
        }
        if(!visited.has(next.code)) {
          visited.add(next.code);
          queue.push([next, cumulative.mul(factor)]);
        }
      }
    }
    return null;
  }

  /**
   * Converts an amount using a currency pair.
   * @param value - The monetary amount to convert.
   * @param pair - The base and counter currencies.
   * @throws CurrencyPairNotFoundError If no conversion exists.
   */
  public convert(value: Money, pair: CurrencyPair): Money;

  /**
   * Converts an amount between two currencies.
   * @param value - The monetary amount to convert.
   * @param base - The base currency.
   * @param counter - The counter currency.
   * @throws CurrencyPairNotFoundError If no conversion exists.
   */
  public convert(value: Money, base: Currency, counter: Currency): Money;
  public convert(value: Money, pair: CurrencyPair | Currency,
      counter?: Currency): Money {
    if(pair instanceof Currency) {
      return this.convert(value, new CurrencyPair(pair, counter));
    }
    const rate = this.find(pair);
    if(!rate) {
      throw new CurrencyPairNotFoundError();
    }
    return rate.convert(value);
  }

  /** Adds or updates an exchange rate. */
  public add(rate: ExchangeRate): void {
    this._directRates.set(rate.pair, rate);
    const inverse = rate.invert();
    this._directRates.set(inverse.pair, inverse);
    this._derivedRates = new Beam.Map<CurrencyPair, ExchangeRate>();
  }

  /** Converts this table to JSON. */
  public toJson(): any {
    return {
      direct_rates: this._directRates.toJson()
    };
  }

  private _directRates: Beam.Map<CurrencyPair, ExchangeRate>;
  private _derivedRates: Beam.Map<CurrencyPair, ExchangeRate>;
}
