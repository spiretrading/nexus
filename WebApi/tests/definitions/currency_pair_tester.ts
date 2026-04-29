import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Currency } from '../../source/definitions/currency';
import { CurrencyPair } from '../../source/definitions/currency_pair';

describe('CurrencyPair', () => {
  it('construct', () => {
    const pair = new CurrencyPair(new Currency(840), new Currency(124));
    assert.ok(pair.base.equals(new Currency(840)));
    assert.ok(pair.counter.equals(new Currency(124)));
  });

  it('none', () => {
    assert.ok(CurrencyPair.NONE.base.equals(Currency.NONE));
    assert.ok(CurrencyPair.NONE.counter.equals(Currency.NONE));
  });

  it('equals', () => {
    const a = new CurrencyPair(new Currency(840), new Currency(124));
    const b = new CurrencyPair(new Currency(840), new Currency(124));
    assert.ok(a.equals(b));
  });

  it('not_equal_base', () => {
    const a = new CurrencyPair(new Currency(840), new Currency(124));
    const b = new CurrencyPair(new Currency(978), new Currency(124));
    assert.ok(!a.equals(b));
  });

  it('not_equal_counter', () => {
    const a = new CurrencyPair(new Currency(840), new Currency(124));
    const b = new CurrencyPair(new Currency(840), new Currency(978));
    assert.ok(!a.equals(b));
  });

  it('invert', () => {
    const original = new CurrencyPair(new Currency(1), new Currency(2));
    const inverted = original.invert();
    assert.ok(inverted.base.equals(new Currency(2)));
    assert.ok(inverted.counter.equals(new Currency(1)));
  });

  it('to_json', () => {
    const pair = new CurrencyPair(new Currency(840), new Currency(124));
    const json = pair.toJson();
    assert.strictEqual(json.base, 840);
    assert.strictEqual(json.counter, 124);
  });

  it('from_json', () => {
    const pair = CurrencyPair.fromJson({ base: 840, counter: 124 });
    assert.ok(pair.base.equals(new Currency(840)));
    assert.ok(pair.counter.equals(new Currency(124)));
  });

  it('round_trip_json', () => {
    const original = new CurrencyPair(new Currency(840), new Currency(124));
    const restored = CurrencyPair.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });

  it('hash', () => {
    const a = new CurrencyPair(new Currency(840), new Currency(124));
    const b = new CurrencyPair(new Currency(840), new Currency(124));
    assert.strictEqual(a.hash(), b.hash());
  });

  it('hash_differs', () => {
    const a = new CurrencyPair(new Currency(840), new Currency(124));
    const b = new CurrencyPair(new Currency(124), new Currency(840));
    assert.notStrictEqual(a.hash(), b.hash());
  });
});
