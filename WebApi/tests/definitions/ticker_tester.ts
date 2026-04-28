import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Ticker } from '../../source/definitions/ticker';
import { Venue } from '../../source/definitions/venue';

describe('Ticker', () => {
  it('construct', () => {
    const ticker = new Ticker('AAPL', new Venue('XNAS'));
    assert.strictEqual(ticker.symbol, 'AAPL');
    assert.ok(ticker.venue.equals(new Venue('XNAS')));
  });

  it('none', () => {
    assert.strictEqual(Ticker.NONE.symbol, '');
    assert.ok(Ticker.NONE.venue.equals(Venue.NONE));
  });

  it('equals', () => {
    const a = new Ticker('AAPL', new Venue('XNAS'));
    const b = new Ticker('AAPL', new Venue('XNAS'));
    assert.ok(a.equals(b));
  });

  it('not_equal_symbol', () => {
    const a = new Ticker('AAPL', new Venue('XNAS'));
    const b = new Ticker('MSFT', new Venue('XNAS'));
    assert.ok(!a.equals(b));
  });

  it('not_equal_venue', () => {
    const a = new Ticker('AAPL', new Venue('XNAS'));
    const b = new Ticker('AAPL', new Venue('XNYS'));
    assert.ok(!a.equals(b));
  });

  it('to_json', () => {
    const ticker = new Ticker('AAPL', new Venue('XNAS'));
    const json = ticker.toJson();
    assert.strictEqual(json.symbol, 'AAPL');
    assert.strictEqual(json.venue, 'XNAS');
  });

  it('from_json', () => {
    const json = { symbol: 'AAPL', venue: 'XNAS' };
    const ticker = Ticker.fromJson(json);
    assert.strictEqual(ticker.symbol, 'AAPL');
    assert.ok(ticker.venue.equals(new Venue('XNAS')));
  });

  it('round_trip_json', () => {
    const original = new Ticker('MSFT', new Venue('XNYS'));
    const restored = Ticker.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });

  it('to_string_no_venue', () => {
    const ticker = new Ticker('AAPL', Venue.NONE);
    assert.strictEqual(ticker.toString(), 'AAPL');
  });

  it('to_string_empty_symbol', () => {
    const ticker = new Ticker('', new Venue('XNAS'));
    assert.strictEqual(ticker.toString(), '');
  });

  it('parse_none_on_no_separator', () => {
    const ticker = Ticker.parse('AAPL');
    assert.ok(ticker.equals(Ticker.NONE));
  });

  it('hash', () => {
    const a = new Ticker('AAPL', new Venue('XNAS'));
    const b = new Ticker('AAPL', new Venue('XNAS'));
    assert.strictEqual(a.hash(), b.hash());
  });

  it('hash_differs', () => {
    const a = new Ticker('AAPL', new Venue('XNAS'));
    const b = new Ticker('AAPL', new Venue('XNYS'));
    assert.notStrictEqual(a.hash(), b.hash());
  });
});
