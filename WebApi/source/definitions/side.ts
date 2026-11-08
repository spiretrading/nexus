import { Quantity } from './quantity';

/** Represents a bid or an ask. */
export enum Side {

  /** No side. */
  NONE = -1,

  /** Ask side. */
  ASK = 0,

  /** Bid side. */
  BID = 1
}

/**
 * Returns a value based on a side.
 * @param side - The side to pick from.
 * @param ask - The value returned for the ask side.
 * @param bid - The value returned for any other side.
 */
export function pick<T>(side: Side, ask: T, bid: T): T {
  if(side === Side.ASK) {
    return ask;
  }
  return bid;
}

/** Returns -1 for ask, 1 for bid, or 0 for no side. */
export function getDirection(side: Side): number {
  if(side === Side.ASK) {
    return -1;
  } else if(side === Side.BID) {
    return 1;
  }
  return 0;
}

/** Returns the side of a quantity. */
export function getSide(quantity: Quantity): Side {
  const direction = quantity.compare(Quantity.ZERO);
  if(direction > 0) {
    return Side.BID;
  } else if(direction < 0) {
    return Side.ASK;
  }
  return Side.NONE;
}

/** Returns the opposite side, or NONE for no side. */
export function getOpposite(side: Side): Side {
  if(side === Side.BID) {
    return Side.ASK;
  } else if(side === Side.ASK) {
    return Side.BID;
  }
  return Side.NONE;
}

/** Returns 'A' for ask, 'B' for bid, or '?' for no side. */
export function toChar(side: Side): string {
  if(side === Side.ASK) {
    return 'A';
  } else if(side === Side.BID) {
    return 'B';
  }
  return '?';
}
