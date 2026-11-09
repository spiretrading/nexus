import * as Beam from 'beam';
import { BboQuote } from '../definitions/bbo_quote';
import { Ticker } from '../definitions/ticker';
import { BboQuoteMessage } from './bbo_quote_message';
import { EndBboQuoteQueryMessage } from './end_bbo_quote_query_message';
import { MarketDataClient } from './market_data_client';
import { QueryBboQuotesRequest } from './query_bbo_quotes_request';
import { SequencedBboQuote, TickerQuery } from './ticker_query';

interface Subscription {
  ticker: Ticker;
  queues: Set<Beam.QueueWriter<BboQuote>>;
  id: number;
  latest: SequencedBboQuote;
  pending: SequencedBboQuote[];
  isInitialized: boolean;
}

/** Implements market data queries using the service protocol over WebSocket. */
export class HttpMarketDataClient extends MarketDataClient {

  /**
   * Constructs a market data client.
   * @param serviceUrl - The market data service WebSocket endpoint.
   */
  constructor(serviceUrl: URL) {
    super();
    this._serviceClient = new Beam.ServiceProtocolClient(serviceUrl);
    this._subscriptions = new Beam.Map<Ticker, Subscription>();
    this._opening = null;
    this._error = null;
    this._serviceClient.monitor(
      BboQuoteMessage, new Beam.CallbackQueueWriter<BboQuoteMessage>(
        this.onBboQuote, this.onBreak));
  }

  public queryBboQuotes(query: TickerQuery, queue: Beam.QueueWriter<BboQuote>):
      Beam.QueueWriter<BboQuote> {
    let subscription: Subscription = null;
    const writer = new Beam.CallbackQueueWriter<BboQuote>(
      value => queue.push(value), error => {
        try {
          queue.close(error);
        } finally {
          if(subscription) {
            this.remove(subscription, writer);
          }
        }
      });
    if(this._error) {
      writer.close(this._error);
      return writer;
    }
    if(!query.equals(Beam.BasicQuery.makeCurrentQuery(query.index))) {
      writer.close(new RangeError('Only current BBO queries are supported.'));
      return writer;
    }
    subscription = this._subscriptions.get(query.index);
    let isNew = false;
    if(!subscription) {
      subscription = {
        ticker: query.index,
        queues: new Set(),
        id: null,
        latest: null,
        pending: [],
        isInitialized: false
      };
      this._subscriptions.set(query.index, subscription);
      isNew = true;
    }
    subscription.queues.add(writer);
    if(subscription.latest) {
      try {
        writer.push(subscription.latest.value);
      } catch(error) {
        this.remove(subscription, writer);
      }
    }
    if(isNew) {
      this.subscribe(subscription);
    }
    return writer;
  }

  public async open(): Promise<void> {
    if(this._error) {
      throw this._error;
    }
    if(!this._opening) {
      this._opening = this._serviceClient.open();
    }
    try {
      await this._opening;
    } catch(error) {
      this.onBreak(error as Error);
      throw error;
    }
  }

  public async close(): Promise<void> {
    this.onBreak(new Beam.PipeBrokenError());
  }

  private async subscribe(subscription: Subscription): Promise<void> {
    try {
      await this.open();
      if(this._error) {
        return;
      }
      if(subscription.queues.size === 0) {
        this._subscriptions.delete(subscription.ticker);
        return;
      }
      const result = await this._serviceClient.sendRequest(
        new QueryBboQuotesRequest(
          Beam.BasicQuery.makeCurrentQuery(subscription.ticker)));
      if(this._error) {
        return;
      }
      subscription.id = result.id;
      if(subscription.queues.size === 0) {
        this.end(subscription);
        return;
      }
      for(const value of result.snapshot) {
        this.publish(subscription, value);
      }
      for(const value of subscription.pending) {
        this.publish(subscription, value);
      }
      subscription.pending = [];
      subscription.isInitialized = true;
      if(result.id === -1) {
        this.fail(subscription, new Beam.PipeBrokenError());
      }
    } catch(error) {
      this.fail(subscription, error as Error);
    }
  }

  private publish(subscription: Subscription, value: SequencedBboQuote): void {
    if(subscription.latest &&
        value.sequence.compare(subscription.latest.sequence) <= 0) {
      return;
    }
    subscription.latest = value;
    for(const queue of Array.from(subscription.queues)) {
      try {
        queue.push(value.value);
      } catch(error) {
        this.remove(subscription, queue);
      }
    }
  }

  private remove(subscription: Subscription,
      queue: Beam.QueueWriter<BboQuote>): void {
    if(!subscription.queues.delete(queue)) {
      return;
    }
    queue.close();
    if(subscription.queues.size === 0 && subscription.id !== null) {
      this.end(subscription);
    }
  }

  private end(subscription: Subscription): void {
    if(this._subscriptions.get(subscription.ticker) !== subscription) {
      return;
    }
    this._subscriptions.delete(subscription.ticker);
    subscription.pending = [];
    if(!this._error && subscription.id !== null && subscription.id !== -1) {
      try {
        this._serviceClient.sendMessage(
          new EndBboQuoteQueryMessage(subscription.ticker, subscription.id));
      } catch(error) {
        this.onBreak(error as Error);
      }
    }
  }

  private fail(subscription: Subscription, error: Error): void {
    this.end(subscription);
    for(const queue of subscription.queues) {
      queue.close(error);
    }
    subscription.queues.clear();
  }

  private onBboQuote = (message: BboQuoteMessage): void => {
    const quote = message.bboQuote;
    const subscription = this._subscriptions.get(quote.value.index);
    if(!subscription) {
      return;
    }
    const value = new Beam.SequencedValue(quote.value.value, quote.sequence);
    if(!subscription.isInitialized) {
      subscription.pending.push(value);
    } else {
      this.publish(subscription, value);
    }
  };

  private onBreak = (error: Error): void => {
    if(this._error) {
      return;
    }
    this._error = error;
    const subscriptions = this._subscriptions;
    this._subscriptions = new Beam.Map<Ticker, Subscription>();
    for(const [, subscription] of subscriptions) {
      this.fail(subscription, error);
    }
    this._serviceClient.close();
  };

  private _serviceClient: Beam.ServiceProtocolClient;
  private _subscriptions: Beam.Map<Ticker, Subscription>;
  private _opening: Promise<void>;
  private _error: Error;
}
