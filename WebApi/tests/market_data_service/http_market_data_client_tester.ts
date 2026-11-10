import * as assert from 'node:assert';
import { afterEach, beforeEach, describe, it } from 'node:test';
import { setImmediate } from 'node:timers/promises';
import * as Beam from 'beam';
import { BboQuote, BboQuoteMessage, EndBboQuoteQueryMessage,
  HttpMarketDataClient, makeAsk, makeBid, Money, Quantity, Ticker } from
  '../../source';

class TestWebSocket {
  public static instances: TestWebSocket[] = [];
  public static created: Beam.Queue<TestWebSocket>;
  public url: string;
  public binaryType: string;
  public onopen: () => void;
  public onerror: () => void;
  public onclose: () => void;
  public onmessage: (event: {data: ArrayBuffer}) => void;
  public requests: Beam.Queue<any>;
  public isClosed: boolean;

  constructor(url: string) {
    this.url = url;
    this.binaryType = '';
    this.onopen = null;
    this.onerror = null;
    this.onclose = null;
    this.onmessage = null;
    this.requests = new Beam.Queue<any>();
    this.isClosed = false;
    TestWebSocket.instances.push(this);
    TestWebSocket.created.push(this);
  }

  public send(buffer: ArrayBuffer): void {
    assert.ok(!this.isClosed);
    assert.strictEqual(new DataView(buffer).getUint32(0, true),
      buffer.byteLength - 4);
    this.requests.push(JSON.parse(new TextDecoder().decode(buffer.slice(4))));
  }

  public receive(value: any): void {
    const payload = new TextEncoder().encode(JSON.stringify(value));
    const buffer = new ArrayBuffer(4 + payload.length);
    new DataView(buffer).setUint32(0, payload.length, true);
    new Uint8Array(buffer, 4).set(payload);
    this.onmessage({data: buffer});
  }

  public respond(request: any, result: any, isException: boolean): void {
    this.receive({
      __type: request.__type.replace('.Request', '.Response'),
      __version: 0,
      request_id: request.request_id,
      is_exception: isException,
      result
    });
  }

  public disconnect(): void {
    this.isClosed = true;
    this.onclose?.();
  }

  public close(): void {
    this.isClosed = true;
  }
}

function makeQuote(price: string, sequence: bigint) {
  return new Beam.SequencedValue(new BboQuote(
    makeBid(new Money(price), new Quantity('100')),
    makeAsk(new Money(price).add(Money.CENT), new Quantity('200')),
    Beam.DateTime.fromJson('20261108T100000')), new Beam.Sequence(sequence));
}

function sendQuote(socket: TestWebSocket, ticker: Ticker,
    quote: Beam.SequencedValue<BboQuote>): void {
  socket.receive(new BboQuoteMessage(new Beam.SequencedValue(
    new Beam.IndexedValue(quote.value, ticker), quote.sequence)).toJson());
}

describe('HttpMarketDataClient', () => {
  const originalWebSocket = globalThis.WebSocket;
  const ticker = Ticker.parse('TST.TSX');
  let clients: HttpMarketDataClient[];

  beforeEach(() => {
    clients = [];
    TestWebSocket.instances = [];
    TestWebSocket.created = new Beam.Queue<TestWebSocket>();
    globalThis.WebSocket = TestWebSocket as unknown as typeof WebSocket;
  });

  afterEach(async () => {
    for(const client of clients) {
      await client.close();
    }
    globalThis.WebSocket = originalWebSocket;
  });

  async function openClient() {
    const client = new HttpMarketDataClient(
      new URL('ws://localhost/api/market_data_service/websocket'));
    clients.push(client);
    const opening = client.open();
    const socket = await TestWebSocket.created.pop();
    socket.onopen();
    await opening;
    return {client, socket};
  }

  it('snapshot_and_updates', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    client.queryBboQuotes(query, queue);
    const request = await socket.requests.pop();
    assert.deepStrictEqual(request, {
      __type: 'Nexus.MarketDataService.QueryBboQuotesService.Request',
      __version: 0,
      request_id: 1,
      parameters: {query: query.toJson()}
    });
    const initial = makeQuote('10', 9007199254740993n);
    const update = makeQuote('11', 9007199254740994n);
    sendQuote(socket, ticker, initial);
    sendQuote(socket, ticker, update);
    socket.respond(request, {id: 42, snapshot: [initial.toJson()]}, false);
    assert.ok((await queue.pop()).equals(initial.value));
    assert.ok((await queue.pop()).equals(update.value));
    sendQuote(socket, ticker, initial);
    sendQuote(socket, ticker, update);
    assert.strictEqual(queue.tryPop(), null);
    const newest = makeQuote('12', 9007199254740995n);
    sendQuote(socket, ticker, newest);
    assert.ok((await queue.pop()).equals(newest.value));
  });

  it('independent_subscriptions_and_cancellation', async () => {
    const {client, socket} = await openClient();
    const first = new Beam.Queue<BboQuote>();
    const second = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    const firstSubscription = client.queryBboQuotes(query, first);
    const request = await socket.requests.pop();
    const initial = makeQuote('10', 1n);
    socket.respond(request, {id: 7, snapshot: [initial.toJson()]}, false);
    await first.pop();
    const secondSubscription = client.queryBboQuotes(query, second);
    const secondRequest = await socket.requests.pop();
    socket.respond(secondRequest, {id: 8, snapshot: [initial.toJson()]}, false);
    assert.ok((await second.pop()).equals(initial.value));
    assert.strictEqual(socket.requests.tryPop(), null);
    const error = new Error('Consumer stopped.');
    firstSubscription.close(error);
    firstSubscription.close();
    assert.deepStrictEqual(await socket.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 7).toJson());
    await assert.rejects(first.pop(), received => received === error);
    assert.throws(() => firstSubscription.push(initial.value),
      received => received === error);
    const update = makeQuote('11', 2n);
    sendQuote(socket, ticker, update);
    assert.ok((await second.pop()).equals(update.value));
    assert.strictEqual(socket.requests.tryPop(), null);
    secondSubscription.close();
    assert.deepStrictEqual(await socket.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 8).toJson());
    secondSubscription.close();
    assert.strictEqual(socket.requests.tryPop(), null);
    await assert.rejects(second.pop(), Beam.PipeBrokenError);
  });

  it('multiple_tickers', async () => {
    const {client, socket} = await openClient();
    const other = Ticker.parse('OTHER.TSX');
    const first = new Beam.Queue<BboQuote>();
    const second = new Beam.Queue<BboQuote>();
    const subscription = client.queryBboQuotes(
      Beam.BasicQuery.makeCurrentQuery(ticker), first);
    client.queryBboQuotes(Beam.BasicQuery.makeCurrentQuery(other), second);
    const request = await socket.requests.pop();
    const otherRequest = await socket.requests.pop();
    const quote = makeQuote('10', 1n);
    socket.respond(request, {id: 1, snapshot: [quote.toJson()]}, false);
    socket.respond(otherRequest, {id: 2, snapshot: []}, false);
    await first.pop();
    sendQuote(socket, other, quote);
    assert.ok((await second.pop()).equals(quote.value));
    assert.strictEqual(first.tryPop(), null);
    subscription.close();
    await socket.requests.pop();
    const update = makeQuote('12', 2n);
    sendQuote(socket, ticker, update);
    sendQuote(socket, other, update);
    assert.ok((await second.pop()).equals(update.value));
    await assert.rejects(first.pop(), Beam.PipeBrokenError);
  });

  it('cancel_during_snapshot', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const subscription = client.queryBboQuotes(
      Beam.BasicQuery.makeCurrentQuery(ticker), queue);
    const request = await socket.requests.pop();
    subscription.close();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    sendQuote(socket, ticker, makeQuote('10', 1n));
    socket.respond(request, {id: 9, snapshot: []}, false);
    assert.deepStrictEqual(await socket.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 9).toJson());
  });

  it('close_before_connection', async () => {
    const client = new HttpMarketDataClient(
      new URL('ws://localhost/api/market_data_service/websocket'));
    clients.push(client);
    const queue = new Beam.Queue<BboQuote>();
    const subscription = client.queryBboQuotes(
      Beam.BasicQuery.makeCurrentQuery(ticker), queue);
    subscription.close();
    const socket = TestWebSocket.instances.at(-1);
    socket.onopen();
    await client.open();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    assert.strictEqual(socket.requests.tryPop(), null);
  });

  it('resubscribe_during_snapshot', async () => {
    const {client, socket} = await openClient();
    const first = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    const subscription = client.queryBboQuotes(query, first);
    const request = await socket.requests.pop();
    subscription.close();
    const second = new Beam.Queue<BboQuote>();
    client.queryBboQuotes(query, second);
    const quote = makeQuote('10', 1n);
    const secondRequest = await socket.requests.pop();
    socket.respond(request, {id: 9, snapshot: [quote.toJson()]}, false);
    assert.deepStrictEqual(await socket.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 9).toJson());
    socket.respond(secondRequest, {id: 10, snapshot: [quote.toJson()]}, false);
    assert.ok((await second.pop()).equals(quote.value));
    assert.strictEqual(socket.requests.tryPop(), null);
    await assert.rejects(first.pop(), Beam.PipeBrokenError);
  });

  it('query_failure', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    const subscription = client.queryBboQuotes(query, queue);
    const request = await socket.requests.pop();
    socket.respond(request, 'Query rejected.', true);
    await assert.rejects(queue.pop(), Beam.ServiceError);
    assert.throws(() => subscription.push(makeQuote('10', 1n).value),
      Beam.ServiceError);
    subscription.close();
    const next = new Beam.Queue<BboQuote>();
    client.queryBboQuotes(query, next);
    const retry = await socket.requests.pop();
    socket.respond(retry, {id: 10, snapshot: []}, false);
    const quote = makeQuote('10', 1n);
    sendQuote(socket, ticker, quote);
    assert.ok((await next.pop()).equals(quote.value));
  });

  it('reconnect_snapshot_and_updates', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    const subscription = client.queryBboQuotes(query, queue);
    const request = await socket.requests.pop();
    const initial = makeQuote('10', 1n);
    socket.respond(request, {id: 7, snapshot: [initial.toJson()]}, false);
    await queue.pop();
    socket.disconnect();
    const replacement = await TestWebSocket.created.pop();
    replacement.onopen();
    const recovery = await replacement.requests.pop();
    const expected = query.clone();
    expected.range = Beam.Range.REAL_TIME;
    assert.deepStrictEqual(recovery.parameters, {query: expected.toJson()});
    const current = makeQuote('12', 3n);
    const update = makeQuote('13', 4n);
    sendQuote(replacement, ticker, update);
    replacement.respond(recovery,
      {id: 8, snapshot: [initial.toJson(), current.toJson()]}, false);
    assert.ok((await queue.pop()).equals(current.value));
    assert.ok((await queue.pop()).equals(update.value));
    await client.open();
    sendQuote(replacement, ticker, current);
    await setImmediate();
    assert.strictEqual(queue.tryPop(), null);
    const late = new Beam.Queue<BboQuote>();
    const lateSubscription = client.queryBboQuotes(query, late);
    const lateRequest = await replacement.requests.pop();
    replacement.respond(lateRequest,
      {id: 9, snapshot: [update.toJson()]}, false);
    assert.ok((await late.pop()).equals(update.value));
    assert.strictEqual(replacement.requests.tryPop(), null);
    subscription.close();
    assert.deepStrictEqual(await replacement.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 8).toJson());
    lateSubscription.close();
    assert.deepStrictEqual(await replacement.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 9).toJson());
  });

  it('cancel_during_recovery', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const subscription = client.queryBboQuotes(
      Beam.BasicQuery.makeCurrentQuery(ticker), queue);
    const request = await socket.requests.pop();
    socket.respond(request, {id: 7, snapshot: []}, false);
    await setImmediate();
    socket.disconnect();
    const replacement = await TestWebSocket.created.pop();
    replacement.onopen();
    const recovery = await replacement.requests.pop();
    subscription.close();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    replacement.respond(recovery, {id: 8, snapshot: []}, false);
    assert.deepStrictEqual(await replacement.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 8).toJson());
    await client.open();
  });

  it('disconnect_during_snapshot', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    client.queryBboQuotes(query, queue);
    await socket.requests.pop();
    socket.disconnect();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    const replacement = await TestWebSocket.created.pop();
    replacement.onopen();
    await client.open();
    const next = new Beam.Queue<BboQuote>();
    client.queryBboQuotes(query, next);
    const request = await replacement.requests.pop();
    const quote = makeQuote('10', 1n);
    replacement.respond(request, {id: 1, snapshot: [quote.toJson()]}, false);
    assert.ok((await next.pop()).equals(quote.value));
  });

  it('close_after_snapshot', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    client.queryBboQuotes(query, queue);
    const request = await socket.requests.pop();
    socket.respond(request,
      {id: 1, snapshot: [makeQuote('10', 1n).toJson()]}, false);
    await queue.pop();
    await client.close();
    await client.close();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    const late = new Beam.Queue<BboQuote>();
    client.queryBboQuotes(query, late);
    await assert.rejects(late.pop(), Beam.PipeBrokenError);
    assert.ok(socket.isClosed);
  });

  it('closed_consumer', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    client.queryBboQuotes(Beam.BasicQuery.makeCurrentQuery(ticker), queue);
    const request = await socket.requests.pop();
    queue.close();
    socket.respond(request,
      {id: 3, snapshot: [makeQuote('10', 1n).toJson()]}, false);
    assert.deepStrictEqual(await socket.requests.pop(),
      new EndBboQuoteQueryMessage(ticker, 3).toJson());
  });

  it('historical_query', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeLatestQuery(ticker);
    query.range = new Beam.Range(
      new Beam.Sequence(10n), new Beam.Sequence(20n));
    query.snapshotLimit = Beam.SnapshotLimit.fromHead(2);
    client.queryBboQuotes(query, queue);
    const request = await socket.requests.pop();
    assert.deepStrictEqual(request.parameters, {query: query.toJson()});
    const first = makeQuote('10', 10n);
    const second = makeQuote('11', 11n);
    socket.respond(request,
      {id: -1, snapshot: [first.toJson(), second.toJson()]}, false);
    assert.ok((await queue.pop()).equals(first.value));
    assert.ok((await queue.pop()).equals(second.value));
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    assert.strictEqual(socket.requests.tryPop(), null);
  });

  it('recover_data_query', async () => {
    const {client, socket} = await openClient();
    const queue = new Beam.Queue<BboQuote>();
    const query = Beam.BasicQuery.makeCurrentQuery(ticker);
    query.interruptionPolicy = Beam.InterruptionPolicy.RECOVER_DATA;
    query.snapshotLimit = Beam.SnapshotLimit.UNLIMITED;
    client.queryBboQuotes(query, queue);
    const request = await socket.requests.pop();
    const initial = makeQuote('10', 9007199254740993n);
    socket.respond(request, {id: 7, snapshot: [initial.toJson()]}, false);
    await queue.pop();
    socket.disconnect();
    const replacement = await TestWebSocket.created.pop();
    replacement.onopen();
    const recovery = await replacement.requests.pop();
    const expected = query.clone();
    expected.range = new Beam.Range(initial.sequence.increment(),
      Beam.Sequence.LAST);
    assert.deepStrictEqual(recovery.parameters, {query: expected.toJson()});
    const missed = makeQuote('11', 9007199254740994n);
    const update = makeQuote('12', 9007199254740995n);
    sendQuote(replacement, ticker, update);
    replacement.respond(recovery, {id: 8, snapshot: [missed.toJson()]}, false);
    assert.ok((await queue.pop()).equals(missed.value));
    assert.ok((await queue.pop()).equals(update.value));
  });

  it('connection_retry', async context => {
    context.mock.timers.enable({apis: ['setTimeout']});
    const client = new HttpMarketDataClient(
      new URL('ws://localhost/api/market_data_service/websocket'));
    clients.push(client);
    const queue = new Beam.Queue<BboQuote>();
    client.queryBboQuotes(Beam.BasicQuery.makeCurrentQuery(ticker), queue);
    const socket = await TestWebSocket.created.pop();
    socket.onerror();
    await setImmediate();
    assert.ok(socket.isClosed);
    assert.strictEqual(queue.tryPop(), null);
    context.mock.timers.tick(1000);
    const replacement = await TestWebSocket.created.pop();
    replacement.onopen();
    const request = await replacement.requests.pop();
    const quote = makeQuote('10', 1n);
    replacement.respond(request, {id: 1, snapshot: [quote.toJson()]}, false);
    assert.ok((await queue.pop()).equals(quote.value));
  });

  it('close_while_connecting', async () => {
    const client = new HttpMarketDataClient(
      new URL('ws://localhost/api/market_data_service/websocket'));
    clients.push(client);
    const opening = assert.rejects(client.open(), Beam.PipeBrokenError);
    const socket = await TestWebSocket.created.pop();
    await client.close();
    await opening;
    assert.ok(socket.isClosed);
    await setImmediate();
    assert.strictEqual(TestWebSocket.created.tryPop(), null);
  });
});
