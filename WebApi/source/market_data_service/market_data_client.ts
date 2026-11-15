import * as Beam from 'beam';
import { BboQuote } from '../definitions/bbo_quote';
import { TickerQuery } from './ticker_query';

/** Client used to access market data services. */
export abstract class MarketDataClient {

  /**
   * Queries a ticker's BBO quotes.
   * @param query - The query to submit.
   * @param queue - Receives quotes and closes on completion, cancellation or
   * failure.
   * @return A queue wrapper whose close() cancels the subscription and closes
   *         the supplied queue. Repeated calls to close() have no effect.
   */
  public abstract queryBboQuotes(
    query: TickerQuery, queue: Beam.QueueWriter<BboQuote>):
      Beam.QueueWriter<BboQuote>;

  /** Connects to the service. */
  public abstract open(): Promise<void>;

  /** Disconnects from the service and closes all subscriptions. */
  public abstract close(): Promise<void>;
}
