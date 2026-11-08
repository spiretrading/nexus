import { Currency } from '../definitions/currency';
import { Money } from '../definitions/money';
import { Quantity } from '../definitions/quantity';
import { getSide as getQuantitySide, Side } from '../definitions/side';
import { Ticker } from '../definitions/ticker';

/** Stores information about a single position. */
export class Position {

  /** Parses a Position from JSON. */
  public static fromJson(value: any): Position {
    return new Position(Ticker.fromJson(value.ticker),
      Currency.fromJson(value.currency), Quantity.fromJson(value.quantity),
      Money.fromJson(value.cost_basis));
  }

  /** The ticker held. */
  public ticker: Ticker;

  /** The position's currency. */
  public currency: Currency;

  /** The quantity of inventory held. */
  public quantity: Quantity;

  /** The total cost of the currently held inventory. */
  public costBasis: Money;

  /** Constructs an empty position. */
  constructor();

  /** Constructs an empty position for a ticker and currency. */
  constructor(ticker: Ticker, currency: Currency);

  /**
   * Constructs a Position.
   * @param ticker - The ticker held.
   * @param currency - The position's currency.
   * @param quantity - The quantity held.
   * @param costBasis - The total cost of the currently held inventory.
   */
  constructor(ticker: Ticker, currency: Currency, quantity: Quantity,
    costBasis: Money);
  constructor(ticker?: Ticker, currency?: Currency, quantity?: Quantity,
      costBasis?: Money) {
    this.ticker = ticker ?? Ticker.NONE;
    this.currency = currency ?? Currency.NONE;
    this.quantity = quantity ?? Quantity.ZERO;
    this.costBasis = costBasis ?? Money.ZERO;
  }

  /** Tests if two positions are equal. */
  public equals(other: Position): boolean {
    return other && this.ticker.equals(other.ticker) &&
      this.currency.equals(other.currency) &&
      this.quantity.equals(other.quantity) &&
      this.costBasis.equals(other.costBasis);
  }

  /** Converts this position to JSON. */
  public toJson(): any {
    return {
      ticker: this.ticker.toJson(),
      currency: this.currency.toJson(),
      quantity: this.quantity.toJson(),
      cost_basis: this.costBasis.toJson()
    };
  }
}

/** Returns the average price of a position, or zero for an empty position. */
export function getAveragePrice(position: Position): Money {
  if(position.quantity.equals(Quantity.ZERO)) {
    return Money.ZERO;
  }
  return position.costBasis.divide(
    position.quantity.toJson() / Quantity.MULTIPLIER);
}

/** Returns the side of a position. */
export function getSide(position: Position): Side {
  return getQuantitySide(position.quantity);
}
