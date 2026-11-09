import * as Beam from 'beam';
import { BboQuote } from '../definitions/bbo_quote';
import { Ticker } from '../definitions/ticker';
import { SequencedTickerBboQuote } from './ticker_query';

/** Publishes a ticker's best bid and ask update. */
export class BboQuoteMessage extends Beam.Message {
  public static readonly TYPE = 'Nexus.MarketDataService.BboQuoteMessage';

  public static fromJson(value: any): BboQuoteMessage {
    const quote = value.bbo_quote;
    return new BboQuoteMessage(new Beam.SequencedValue(
      Beam.IndexedValue.fromJson<BboQuote, Ticker>(BboQuote, Ticker,
        quote.value), Beam.Sequence.fromJson(quote.sequence)));
  }

  /** The quote with its ticker and sequence number. */
  public readonly bboQuote: SequencedTickerBboQuote;

  /** Constructs a message containing a BBO update. */
  constructor(bboQuote: SequencedTickerBboQuote) {
    super(BboQuoteMessage.TYPE, {bbo_quote: bboQuote.toJson()});
    this.bboQuote = bboQuote;
  }
}
