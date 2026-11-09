import * as Beam from 'beam';
import { Ticker } from '../definitions/ticker';

/** Cancels a BBO query. */
export class EndBboQuoteQueryMessage extends Beam.Message {
  public static readonly TYPE =
    'Nexus.MarketDataService.EndBboQuoteQueryMessage';

  public static fromJson(value: any): EndBboQuoteQueryMessage {
    return new EndBboQuoteQueryMessage(Ticker.fromJson(value.ticker), value.id);
  }

  /** The ticker that was queried. */
  public readonly ticker: Ticker;

  /** The query ID returned by the service. */
  public readonly id: number;

  /** Constructs a cancellation for a ticker and query ID. */
  constructor(ticker: Ticker, id: number) {
    super(EndBboQuoteQueryMessage.TYPE, {ticker: ticker.toJson(), id});
    this.ticker = ticker;
    this.id = id;
  }
}
