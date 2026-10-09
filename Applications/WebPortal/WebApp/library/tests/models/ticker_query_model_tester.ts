import * as Nexus from 'nexus';
import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { HttpTickerQueryModel, LocalTickerQueryModel } from
  '../../source/models';

describe('TickerQueryModel', () => {
  it('qualified_tickers', async () => {
    const model = new LocalTickerQueryModel([]);
    const ticker = Nexus.Ticker.parse('ABX.TSX');
    assert.ok((await model.parse(' abx.tsx ')).equals(ticker));
    assert.ok((await model.parse('ABX.XTSE')).equals(ticker));
    assert.equal((await model.parse('teck.b.tsx')).toString(), 'TECK.B.TSX');
    assert.equal(
      (await model.parse('UNLISTED.TSX')).toString(), 'UNLISTED.TSX');
    for(const query of ['', 'ABX', 'ABX.', '.TSX', 'ABX.UNKNOWN', 'A B.TSX']) {
      assert.equal(await model.parse(query), null, query);
    }
  });

  it('prefixes_and_venue_aliases', async () => {
    const first = Nexus.Ticker.parse('BMO.TSX');
    const second = Nexus.Ticker.parse('BMO.ASX');
    const other = Nexus.Ticker.parse('ABX.TSX');
    const model = new LocalTickerQueryModel([
      first, second, other, Nexus.Ticker.parse('BMO.TSX')]);
    assert.deepEqual(await model.submit(' bmo '), [first, second]);
    assert.deepEqual(await model.submit('BMO.T'), [first]);
    assert.deepEqual(await model.submit('bmo.xt'), [first]);
    assert.deepEqual(await model.submit('MO'), []);
    assert.deepEqual(await model.submit(' '), []);
  });

  it('http_lookup_and_errors', async () => {
    const original = globalThis.XMLHttpRequest;
    const requests: {method: string; url: string; body: string}[] = [];
    let status = 200;
    let response = JSON.stringify([
      {ticker: Nexus.Ticker.parse('SHOP.TSX').toJson()},
      {ticker: Nexus.Ticker.parse('ABX.TSX').toJson()}
    ]);
    globalThis.XMLHttpRequest = class {
      public open(method: string, url: string): void {
        this.request = {method, url, body: ''};
      }

      public setRequestHeader(): void {}

      public send(body: string): void {
        this.request.body = body;
        requests.push(this.request);
        this.status = status;
        this.responseText = response;
        this.onload();
      }

      public onload: () => void;
      public status: number;
      public responseText: string;
      private request: {method: string; url: string; body: string};
    } as unknown as typeof XMLHttpRequest;
    try {
      const model = new HttpTickerQueryModel();
      assert.deepEqual(await model.submit(' '), []);
      assert.equal(requests.length, 0);
      assert.ok((await model.parse('ABX.XTSE')).equals(
        Nexus.Ticker.parse('ABX.TSX')));
      assert.equal(requests.length, 0);
      assert.deepEqual((await model.submit(' ab ')).map(t => t.toString()),
        ['SHOP.TSX', 'ABX.TSX']);
      assert.deepEqual(requests[0], {method: 'POST',
        url: '/api/market_data_service/load_ticker_info_from_prefix',
        body: JSON.stringify({prefix: 'AB'})});
      status = 503;
      response = JSON.stringify({message: 'Unavailable'});
      await assert.rejects(model.submit('ABX'), /Unavailable/);
    } finally {
      globalThis.XMLHttpRequest = original;
    }
  });

  it('http_venue_aliases_and_dotted_symbols', async () => {
    const original = globalThis.XMLHttpRequest;
    const tickers = ['BMO.TSX', 'BMO.TSXV', 'BMO.TSX.ASX', 'TECK.B.TSX'].
      map(value => Nexus.Ticker.parse(value));
    const requests: string[] = [];
    globalThis.XMLHttpRequest = class {
      public open(): void {}

      public setRequestHeader(): void {}

      public send(body: string): void {
        const {prefix} = JSON.parse(body);
        requests.push(prefix);
        this.status = 200;
        this.responseText = JSON.stringify(tickers.filter(ticker =>
          `${ticker.symbol}.${ticker.venue}`.startsWith(prefix)).
          map(ticker => ({ticker: ticker.toJson()})));
        this.onload();
      }

      public onload: () => void;
      public status: number;
      public responseText: string;
    } as unknown as typeof XMLHttpRequest;
    try {
      const model = new HttpTickerQueryModel();
      for(const query of ['BMO.T', ' bmo.tsx ']) {
        const results = (await model.submit(query)).map(t => t.toString());
        assert.deepEqual(results.sort(),
          ['BMO.TSX', 'BMO.TSX.ASX', 'BMO.TSXV']);
      }
      assert.deepEqual((await model.submit('BMO.XTSE')).map(t => t.toString()),
        ['BMO.TSX']);
      assert.deepEqual((await model.submit('TECK.B.T')).map(t => t.toString()),
        ['TECK.B.TSX']);
      requests.length = 0;
      assert.equal((await model.submit('BMO.')).length, 3);
      assert.deepEqual(requests, ['BMO.']);
    } finally {
      globalThis.XMLHttpRequest = original;
    }
  });
});
