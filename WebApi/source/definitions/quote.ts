import { Money } from './money';
import { Quantity } from './quantity';
import { Side } from './side';

/** Represents a quote on a ticker. */
export class Quote {

  /** Parses a Quote from JSON. */
  public static fromJson(value: any): Quote {
    return new Quote(Money.fromJson(value.price), Quantity.fromJson(value.size),
      value.side);
  }

  /** The quote's price. */
  public price: Money;

  /** The quote's size. */
  public size: Quantity;

  /** The quote's side. */
  public side: Side;

  /** Constructs an empty quote with no side. */
  constructor();

  /**
   * Constructs a Quote.
   * @param price - The quote's price.
   * @param size - The quote's size.
   * @param side - The quote's side.
   */
  constructor(price: Money, size: Quantity, side: Side);
  constructor(price?: Money, size?: Quantity, side?: Side) {
    this.price = price ?? Money.ZERO;
    this.size = size ?? Quantity.ZERO;
    this.side = side ?? Side.NONE;
  }

  /** Tests if two quotes are equal. */
  public equals(other: Quote): boolean {
    return other && this.price.equals(other.price) &&
      this.size.equals(other.size) && this.side === other.side;
  }

  /** Converts this quote to JSON. */
  public toJson(): any {
    return {
      price: this.price.toJson(),
      size: this.size.toJson(),
      side: this.side
    };
  }
}

/**
 * Constructs an ask quote.
 * @param price - The ask's price.
 * @param size - The size being asked.
 */
export function makeAsk(price: Money, size: Quantity): Quote {
  return new Quote(price, size, Side.ASK);
}

/**
 * Constructs a bid quote.
 * @param price - The bid's price.
 * @param size - The size being bid.
 */
export function makeBid(price: Money, size: Quantity): Quote {
  return new Quote(price, size, Side.BID);
}

/**
 * Tests if one quote precedes another by side and price.
 * @param lhs - The quote whose side determines the ordering.
 * @param rhs - The quote to compare against.
 * @return Whether lhs precedes rhs.
 */
export function listingComparator(lhs: Quote, rhs: Quote): boolean {
  if(lhs.side === Side.ASK) {
    return lhs.price.compare(rhs.price) < 0;
  }
  return lhs.price.compare(rhs.price) > 0;
}

/**
 * Compares the precedence of two offer prices for a side.
 * @param side - The side of the offers.
 * @param lhs - The left price.
 * @param rhs - The right price.
 * @return -1 if lhs precedes rhs, 0 if equal, or 1 if rhs precedes lhs.
 */
export function offerComparator(side: Side, lhs: Money, rhs: Money): number {
  const comparison = lhs.compare(rhs);
  if(side === Side.ASK) {
    if(comparison < 0) {
      return -1;
    } else if(comparison === 0) {
      return 0;
    }
    return 1;
  }
  if(comparison > 0) {
    return -1;
  } else if(comparison === 0) {
    return 0;
  }
  return 1;
}
