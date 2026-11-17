import * as Beam from 'beam';
import { BboQuote } from '../definitions/bbo_quote';
import { Ticker } from '../definitions/ticker';
import { BboQuoteMessage } from './bbo_quote_message';
import { EndBboQuoteQueryMessage } from './end_bbo_quote_query_message';
import { MarketDataClient } from './market_data_client';
import { QueryBboQuotesRequest } from './query_bbo_quotes_request';
import { TickerQuery } from './ticker_query';

/** Implements market data queries using the service protocol over WebSocket. */
export class HttpMarketDataClient extends MarketDataClient {

  /**
   * Constructs a market data client.
   * @param serviceUrl - The market data service WebSocket endpoint.
   */
  constructor(serviceUrl: URL) {
    super();
    this._clientHandler = new Beam.ServiceProtocolClientHandler(
      serviceUrl, client => this._publisher.recover(client));
    this._publisher = new Beam.QueryClientPublisher<Ticker, BboQuote>(
      this._clientHandler, query => new QueryBboQuotesRequest(query),
      (ticker, id) => new EndBboQuoteQueryMessage(ticker, id),
      filter => Beam.translate<boolean>(filter));
    this._publisher.addMessageHandler(
      BboQuoteMessage, message => message.bboQuote);
  }

  public queryBboQuotes(query: TickerQuery, queue: Beam.QueueWriter<BboQuote>):
      Beam.QueueWriter<BboQuote> {
    return this._publisher.submit(query, queue);
  }

  /**
   * Connects to the service, retrying connection failures until closed.
   * Waits for established subscriptions to recover when reconnecting.
   */
  public async open(): Promise<void> {
    await this._clientHandler.getClient();
  }

  public async close(): Promise<void> {
    this._publisher.close();
    this._clientHandler.close();
  }

  private _clientHandler: Beam.ServiceProtocolClientHandler;
  private _publisher: Beam.QueryClientPublisher<Ticker, BboQuote>;
}
