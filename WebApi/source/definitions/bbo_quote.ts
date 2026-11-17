import * as Beam from 'beam';
import { Money } from './money';
import { Quantity } from './quantity';
import { makeAsk, makeBid, Quote } from './quote';

/** Stores the best bid and ask quotes across all venues. */
export class BboQuote {

  /** Parses a BboQuote from JSON. */
  public static fromJson(value: any): BboQuote {
    return new BboQuote(Quote.fromJson(value.bid), Quote.fromJson(value.ask),
      Beam.DateTime.fromJson(value.timestamp));
  }

  /** The best bid. */
  public bid: Quote;

  /** The best ask. */
  public ask: Quote;

  /** The time of the update. */
  public timestamp: Beam.DateTime;

  /** Constructs empty bid and ask quotes with no timestamp. */
  constructor();

  /**
   * Constructs a BboQuote.
   * @param bid - The best quote with side BID.
   * @param ask - The best quote with side ASK.
   * @param timestamp - The time of the update.
   */
  constructor(bid: Quote, ask: Quote, timestamp: Beam.DateTime);
  constructor(bid?: Quote, ask?: Quote, timestamp?: Beam.DateTime) {
    this.bid = bid ?? makeBid(Money.ZERO, Quantity.ZERO);
    this.ask = ask ?? makeAsk(Money.ZERO, Quantity.ZERO);
    this.timestamp = timestamp ?? Beam.DateTime.NOT_A_DATE_TIME;
  }

  /** Tests if two best bid and ask updates are equal. */
  public equals(other: BboQuote): boolean {
    return other && this.bid.equals(other.bid) && this.ask.equals(other.ask) &&
      this.timestamp.equals(other.timestamp);
  }

  /** Converts this best bid and ask update to JSON. */
  public toJson(): any {
    return {
      bid: this.bid.toJson(),
      ask: this.ask.toJson(),
      timestamp: this.timestamp.toJson()
    };
  }
}
