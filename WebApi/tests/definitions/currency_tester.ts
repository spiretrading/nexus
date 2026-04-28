import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Currency, CurrencyDatabase } from '../../source/definitions/currency';

describe('Currency', () => {
  it('construct', () => {
    const currency = new Currency(840);
    assert.strictEqual(currency.code, 840);
  });

  it('default_construct', () => {
    const currency = new Currency();
    assert.strictEqual(currency.code, 0);
  });

  it('none', () => {
    assert.strictEqual(Currency.NONE.code, 0);
  });

  it('equals', () => {
    const a = new Currency(100);
    const b = new Currency(100);
    assert.ok(a.equals(b));
  });

  it('not_equal', () => {
    const a = new Currency(100);
    const b = new Currency(50);
    assert.ok(!a.equals(b));
  });

  it('to_json', () => {
    assert.strictEqual(new Currency(840).toJson(), 840);
  });

  it('from_json', () => {
    const currency = Currency.fromJson(840);
    assert.strictEqual(currency.code, 840);
  });

  it('round_trip_json', () => {
    const original = new Currency(124);
    const restored = Currency.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });

  it('hash', () => {
    const a = new Currency(840);
    const b = new Currency(840);
    assert.strictEqual(a.hash(), b.hash());
  });
});

describe('CurrencyDatabase', () => {
  it('none_entry', () => {
    const none = CurrencyDatabase.Entry.NONE;
    assert.ok(none.currency.equals(Currency.NONE));
    assert.strictEqual(none.code, '?');
    assert.strictEqual(none.sign, '????');
  });

  it('currency_lookup', () => {
    const database = new CurrencyDatabase();
    database.add(new CurrencyDatabase.Entry(
      new Currency(10), 'AAA', 'A$'));
    database.add(new CurrencyDatabase.Entry(
      new Currency(5), 'BBB', 'B$'));
    assert.strictEqual(
      database.fromCurrency(new Currency(10)).sign, 'A$');
    assert.strictEqual(
      database.fromCurrency(new Currency(5)).sign, 'B$');
    assert.ok(
      database.fromCurrency(new Currency(7)).currency.equals(Currency.NONE));
  });

  it('code_lookup', () => {
    const database = new CurrencyDatabase();
    database.add(new CurrencyDatabase.Entry(
      new Currency(20), 'CCC', 'C$'));
    assert.ok(
      database.fromCode('CCC').currency.equals(new Currency(20)));
    assert.ok(
      database.fromCode('XXX').currency.equals(Currency.NONE));
  });

  it('remove', () => {
    const database = new CurrencyDatabase();
    database.add(new CurrencyDatabase.Entry(
      new Currency(30), 'DDD', 'D$'));
    assert.ok(
      database.fromCurrency(new Currency(30)).currency.equals(
        new Currency(30)));
    database.delete(new Currency(30));
    assert.ok(
      database.fromCurrency(new Currency(30)).currency.equals(Currency.NONE));
  });

  it('add_duplicate_ignored', () => {
    const database = new CurrencyDatabase();
    database.add(new CurrencyDatabase.Entry(
      new Currency(10), 'AAA', 'A$'));
    database.add(new CurrencyDatabase.Entry(
      new Currency(20), 'AAA', 'X$'));
    assert.strictEqual(
      database.fromCode('AAA').sign, 'A$');
  });

  it('entry_equals', () => {
    const a = new CurrencyDatabase.Entry(new Currency(840), 'USD', '$');
    const b = new CurrencyDatabase.Entry(new Currency(840), 'USD', '$');
    assert.ok(a.equals(b));
  });

  it('entry_not_equal', () => {
    const a = new CurrencyDatabase.Entry(new Currency(840), 'USD', '$');
    const b = new CurrencyDatabase.Entry(new Currency(124), 'CAD', 'C$');
    assert.ok(!a.equals(b));
  });

  it('entry_to_json', () => {
    const entry = new CurrencyDatabase.Entry(new Currency(840), 'USD', '$');
    const json = entry.toJson();
    assert.strictEqual(json.id, 840);
    assert.strictEqual(json.code, 'USD');
    assert.strictEqual(json.sign, '$');
  });

  it('entry_from_json', () => {
    const entry = CurrencyDatabase.Entry.fromJson({
      id: 840, code: 'USD', sign: '$'
    });
    assert.ok(entry.currency.equals(new Currency(840)));
    assert.strictEqual(entry.code, 'USD');
    assert.strictEqual(entry.sign, '$');
  });

  it('round_trip_json', () => {
    const database = new CurrencyDatabase();
    database.add(new CurrencyDatabase.Entry(new Currency(840), 'USD', '$'));
    database.add(new CurrencyDatabase.Entry(new Currency(124), 'CAD', 'C$'));
    const json = { entries: database.toJson() };
    const restored = CurrencyDatabase.fromJson(json);
    assert.strictEqual(
      restored.fromCode('USD').currency.code, 840);
    assert.strictEqual(
      restored.fromCode('CAD').currency.code, 124);
  });

  it('iterable', () => {
    const database = new CurrencyDatabase();
    database.add(new CurrencyDatabase.Entry(new Currency(1), 'AAA', 'A'));
    database.add(new CurrencyDatabase.Entry(new Currency(2), 'BBB', 'B'));
    const entries = [...database];
    assert.strictEqual(entries.length, 2);
  });
});
