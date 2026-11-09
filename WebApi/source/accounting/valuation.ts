import { Currency } from '../definitions/currency';
import { Money } from '../definitions/money';

/** Stores the valuation used for a ticker. */
export class Valuation {

  /** Parses a Valuation from JSON. */
  public static fromJson(value: any): Valuation {
    const valuation = new Valuation(Currency.fromJson(value.currency));
    if(value.ask_value.is_initialized) {
      valuation.askValue = Money.fromJson(value.ask_value.value);
    }
    if(value.bid_value.is_initialized) {
      valuation.bidValue = Money.fromJson(value.bid_value.value);
    }
    return valuation;
  }

  /** The currency used to value the ticker. */
  public currency: Currency;

  /** The ask side value, or null when unavailable. */
  public askValue: Money;

  /** The bid side value, or null when unavailable. */
  public bidValue: Money;

  /** Constructs a valuation with no currency or prices. */
  constructor();

  /**
   * Constructs a valuation with no prices.
   * @param currency - The currency used to value the ticker.
   */
  constructor(currency: Currency);
  constructor(currency?: Currency) {
    this.currency = currency ?? Currency.NONE;
    this.askValue = null;
    this.bidValue = null;
  }

  /** Converts this valuation to JSON. */
  public toJson(): any {
    return {
      currency: this.currency.toJson(),
      ask_value: toOptionalMoneyJson(this.askValue),
      bid_value: toOptionalMoneyJson(this.bidValue)
    };
  }
}

function toOptionalMoneyJson(value: Money): any {
  if(!value) {
    return {is_initialized: false};
  }
  return {is_initialized: true, value: value.toJson()};
}
