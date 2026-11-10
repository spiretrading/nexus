import * as assert from 'node:assert';
import { afterEach, beforeEach, describe, it } from 'node:test';
import * as Beam from 'beam';
import { Inventory, Money, Position, Quantity, Ticker } from '../../source';
import { Currencies } from '../../source/definitions/standard_currencies';
import { RiskClient } from '../../source/risk_service/risk_client';
import { HttpRiskClient } from '../../source/risk_service/http_risk_client';
import { InventoryMessage } from '../../source/risk_service/inventory_message';
import { InventoryUpdate } from '../../source/risk_service/inventory_update';
import { RiskInventoryEntry, RiskPortfolioKey } from
  '../../source/risk_service/risk_portfolio_types';

class TestWebSocket {
  public static instances: TestWebSocket[] = [];
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

function makeEntry(account: number, quantity: string,
    count: number): RiskInventoryEntry {
  const ticker = Ticker.parse('TST.TSX');
  return new RiskInventoryEntry(new RiskPortfolioKey(
    Beam.DirectoryEntry.makeAccount(account, 'trader'), ticker),
    new Inventory(new Position(ticker, Currencies.CAD,
      new Quantity(quantity), new Money('100')), Money.ZERO, Money.ZERO,
      new Quantity(quantity), count));
}

function sendUpdates(socket: TestWebSocket,
    entries: RiskInventoryEntry[]): void {
  socket.receive(new InventoryMessage(entries.map(entry =>
    new InventoryUpdate(entry.key.account, entry.value))).toJson());
}

describe('HttpRiskClient', () => {
  const originalWebSocket = globalThis.WebSocket;
  let clients: HttpRiskClient[];

  beforeEach(() => {
    clients = [];
    TestWebSocket.instances = [];
    globalThis.WebSocket = TestWebSocket as unknown as typeof WebSocket;
  });

  afterEach(async () => {
    for(const client of clients) {
      await client.close();
    }
    globalThis.WebSocket = originalWebSocket;
  });

  async function openClient() {
    const client = new HttpRiskClient(
      new URL('ws://localhost/api/risk_service/websocket'));
    clients.push(client);
    const opening = client.open();
    const socket = TestWebSocket.instances.at(-1);
    socket.onopen();
    await opening;
    return {client, socket};
  }

  it('snapshot_and_updates', async () => {
    const {client, socket} = await openClient();
    const riskClient: RiskClient = client;
    const publisher = riskClient.getRiskPortfolioUpdatePublisher();
    assert.strictEqual(publisher.getSnapshot().size, 0);
    const queue = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(queue);
    assert.strictEqual(client.getRiskPortfolioUpdatePublisher(), publisher);
    const request = await socket.requests.pop();
    assert.deepStrictEqual(request, {
      __type: 'Nexus.RiskService.SubscribeRiskPortfolioUpdatesService.Request',
      __version: 0,
      request_id: 1
    });
    const first = makeEntry(1, '10', 1);
    const second = makeEntry(2, '-5', 2);
    const update = makeEntry(1, '20', 3);
    socket.respond(request, [first.toJson(), second.toJson()], false);
    sendUpdates(socket, [update]);
    assert.ok((await queue.pop()).equals(first));
    assert.ok((await queue.pop()).equals(second));
    assert.ok((await queue.pop()).equals(update));
    const late = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(late);
    const snapshot = [await late.pop(), await late.pop()];
    assert.ok(snapshot.some(entry => entry.equals(update)));
    assert.ok(snapshot.some(entry => entry.equals(second)));
    const captured = {value: new Beam.Map<RiskPortfolioKey, Inventory>()};
    const subsequent = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(subsequent, captured);
    assert.strictEqual(captured.value.size, 2);
    assert.ok(captured.value.get(update.key).equals(update.value));
    assert.ok(captured.value.get(second.key).equals(second.value));
    assert.strictEqual(subsequent.tryPop(), null);
    const copy = publisher.getSnapshot();
    copy.delete(update.key);
    assert.strictEqual(publisher.getSnapshot().size, 2);
    const newest = makeEntry(2, '-10', 4);
    sendUpdates(socket, [newest]);
    assert.ok((await queue.pop()).equals(newest));
    assert.ok((await late.pop()).equals(newest));
    const published = await subsequent.pop();
    assert.ok(published instanceof RiskInventoryEntry);
    assert.ok(published.equals(newest));
    assert.ok(publisher.getSnapshot().get(newest.key).equals(newest.value));
    assert.ok(captured.value.get(second.key).equals(second.value));
    assert.strictEqual(socket.requests.tryPop(), null);
  });

  it('removals_and_empty_snapshot_entries', async () => {
    const {client, socket} = await openClient();
    const publisher = client.getRiskPortfolioUpdatePublisher();
    const queue = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(queue);
    const request = await socket.requests.pop();
    const empty = makeEntry(1, '0', 0);
    empty.value = new Inventory(empty.key.ticker, Currencies.CAD);
    const position = makeEntry(2, '10', 0);
    const closed = makeEntry(3, '0', 5);
    socket.respond(request,
      [empty.toJson(), position.toJson(), closed.toJson()], false);
    assert.ok((await queue.pop()).equals(position));
    assert.ok((await queue.pop()).equals(closed));
    assert.strictEqual(queue.tryPop(), null);
    const removed = new RiskInventoryEntry(position.key,
      new Inventory(position.key.ticker, Currencies.CAD));
    sendUpdates(socket, [removed, empty]);
    const deletion = await queue.pop();
    assert.ok(deletion instanceof RiskInventoryEntry);
    assert.ok(deletion.equals(removed));
    assert.strictEqual(queue.tryPop(), null);
    assert.strictEqual(publisher.getSnapshot().size, 1);
    assert.ok(!publisher.getSnapshot().has(position.key));
    assert.ok(publisher.getSnapshot().get(closed.key).equals(closed.value));
    const late = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(late);
    assert.ok((await late.pop()).equals(closed));
    assert.strictEqual(late.tryPop(), null);
  });

  it('subscription_failure', async () => {
    const {client, socket} = await openClient();
    const publisher = client.getRiskPortfolioUpdatePublisher();
    const queue = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(queue);
    const request = await socket.requests.pop();
    socket.respond(request, 'Subscription rejected.', true);
    await assert.rejects(queue.pop(), Beam.ServiceError);
    const late = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(late);
    await assert.rejects(late.pop(), Beam.ServiceError);
    assert.ok(socket.isClosed);
  });

  it('disconnect_during_snapshot', async () => {
    const {client, socket} = await openClient();
    const publisher = client.getRiskPortfolioUpdatePublisher();
    const queue = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(queue);
    await socket.requests.pop();
    socket.disconnect();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    const late = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(late);
    await assert.rejects(late.pop(), Beam.PipeBrokenError);
  });

  it('close_after_snapshot', async () => {
    const {client, socket} = await openClient();
    const publisher = client.getRiskPortfolioUpdatePublisher();
    const queue = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(queue);
    const request = await socket.requests.pop();
    const entry = makeEntry(1, '10', 1);
    socket.respond(request, [entry.toJson()], false);
    await queue.pop();
    await client.close();
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    const late = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(late);
    assert.ok((await late.pop()).equals(entry));
    await assert.rejects(late.pop(), Beam.PipeBrokenError);
    assert.ok(publisher.getSnapshot().get(entry.key).equals(entry.value));
    const captured = {value: new Beam.Map<RiskPortfolioKey, Inventory>()};
    const subsequent = new Beam.Queue<RiskInventoryEntry>();
    publisher.monitor(subsequent, captured);
    assert.ok(captured.value.get(entry.key).equals(entry.value));
    await assert.rejects(subsequent.pop(), Beam.PipeBrokenError);
    assert.ok(socket.isClosed);
    await client.close();
  });

  it('disconnect_before_subscription', async () => {
    const {client, socket} = await openClient();
    socket.disconnect();
    const queue = new Beam.Queue<RiskInventoryEntry>();
    client.getRiskPortfolioUpdatePublisher().monitor(queue);
    await assert.rejects(queue.pop(), Beam.PipeBrokenError);
    assert.strictEqual(socket.requests.tryPop(), null);
  });
});
