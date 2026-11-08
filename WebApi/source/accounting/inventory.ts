import { Currency } from '../definitions/currency';
import { Money } from '../definitions/money';
import { Quantity } from '../definitions/quantity';
import { Ticker } from '../definitions/ticker';
import { Position } from './position';

/** Stores bookkeeping information for a single inventory. */
export class Inventory {

  /** Parses an Inventory from JSON. */
  public static fromJson(value: any): Inventory {
    return new Inventory(Position.fromJson(value.position),
      Money.fromJson(value.gross_profit_and_loss), Money.fromJson(value.fees),
      Quantity.fromJson(value.volume), value.transaction_count);
  }

  /** The currently held position. */
  public position: Position;

  /** The inventory's gross profit and loss. */
  public grossProfitAndLoss: Money;

  /** The transaction fees. */
  public fees: Money;

  /** The total quantity that was transacted. */
  public volume: Quantity;

  /** The number of transactions made. */
  public transactionCount: number;

  /** Constructs an empty inventory. */
  constructor();

  /** Constructs an empty inventory for a ticker and currency. */
  constructor(ticker: Ticker, currency: Currency);

  /**
   * Constructs an Inventory.
   * @param position - The currently held position.
   * @param grossProfitAndLoss - The inventory's gross profit and loss.
   * @param fees - The transaction fees.
   * @param volume - The total quantity that was transacted.
   * @param transactionCount - The number of transactions made.
   */
  constructor(position: Position, grossProfitAndLoss: Money, fees: Money,
    volume: Quantity, transactionCount: number);
  constructor(position?: Position | Ticker,
      currencyOrProfitAndLoss?: Currency | Money, fees?: Money,
      volume?: Quantity, transactionCount?: number) {
    if(position instanceof Position) {
      this.position = position;
      this.grossProfitAndLoss = currencyOrProfitAndLoss as Money;
      this.fees = fees;
      this.volume = volume;
      this.transactionCount = transactionCount;
    } else {
      this.position = new Position(position ?? Ticker.NONE,
        (currencyOrProfitAndLoss as Currency) ?? Currency.NONE);
      this.grossProfitAndLoss = Money.ZERO;
      this.fees = Money.ZERO;
      this.volume = Quantity.ZERO;
      this.transactionCount = 0;
    }
  }

  /** Tests if two inventories are equal. */
  public equals(other: Inventory): boolean {
    return other && this.position.equals(other.position) &&
      this.grossProfitAndLoss.equals(other.grossProfitAndLoss) &&
      this.fees.equals(other.fees) && this.volume.equals(other.volume) &&
      this.transactionCount === other.transactionCount;
  }

  /** Converts this inventory to JSON. */
  public toJson(): any {
    return {
      position: this.position.toJson(),
      gross_profit_and_loss: this.grossProfitAndLoss.toJson(),
      fees: this.fees.toJson(),
      volume: this.volume.toJson(),
      transaction_count: this.transactionCount
    };
  }
}

/** Tests if an inventory has no position or bookkeeping activity. */
export function isEmpty(inventory: Inventory): boolean {
  return inventory.equals(new Inventory(
    inventory.position.ticker, inventory.position.currency));
}
