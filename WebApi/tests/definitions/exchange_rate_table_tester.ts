import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import Fraction from 'fraction.js';
import { Currency } from '../../source/definitions/currency';
import { CurrencyPair } from '../../source/definitions/currency_pair';
import { CurrencyPairNotFoundError } from
  '../../source/definitions/currency_pair_not_found_error';
import { ExchangeRate } from '../../source/definitions/exchange_rate';
import { ExchangeRateTable } from
  '../../source/definitions/exchange_rate_table';
import { Money } from '../../source/definitions/money';
import { Currencies } from '../../source/definitions/standard_currencies';

describe('ExchangeRateTable', () => {
  it('identity_and_missing_pair', () => {
    const table = new ExchangeRateTable();
    const pair = new CurrencyPair(Currencies.USD, Currencies.EUR);
    assert.strictEqual(table.find(pair), null);
    assert.throws(() => table.convert(Money.ONE, pair),
      CurrencyPairNotFoundError);
    assert.throws(() => table.convert(Money.ONE, Currencies.USD,
      Currencies.EUR), CurrencyPairNotFoundError);
    assert.ok(table.convert(Money.ONE, Currencies.USD, Currencies.USD).
      equals(Money.ONE));
  });

  it('direct_and_inverse', () => {
    const table = new ExchangeRateTable([
      new ExchangeRate(new CurrencyPair(Currencies.USD, Currencies.CAD),
        new Fraction(3, 2))
    ]);
    const pair = new CurrencyPair(new Currency(840), new Currency(124));
    assert.ok(table.find(pair).rate.equals(new Fraction(3, 2)));
    assert.ok(table.convert(new Money('100'), pair).equals(new Money('150')));
    assert.ok(table.convert(new Money('150'), Currencies.CAD, Currencies.USD).
      equals(new Money('100')));
  });

  it('chained_conversion_and_disconnected_currency', () => {
    const table = new ExchangeRateTable([
      new ExchangeRate(new CurrencyPair(Currencies.USD, Currencies.CAD),
        new Fraction(2)),
      new ExchangeRate(new CurrencyPair(Currencies.EUR, Currencies.CAD),
        new Fraction(1, 3)),
      new ExchangeRate(new CurrencyPair(Currencies.EUR, Currencies.GBP),
        new Fraction(5))
    ]);
    assert.ok(table.convert(new Money('7'), Currencies.USD, Currencies.GBP).
      equals(new Money('210')));
    assert.ok(table.convert(new Money('210'), Currencies.GBP, Currencies.USD).
      equals(new Money('7')));
    assert.strictEqual(table.find(
      new CurrencyPair(Currencies.USD, Currencies.JPY)), null);
  });

  it('rate_update_and_direct_precedence', () => {
    const table = new ExchangeRateTable([
      new ExchangeRate(new CurrencyPair(Currencies.USD, Currencies.CAD),
        new Fraction(2)),
      new ExchangeRate(new CurrencyPair(Currencies.CAD, Currencies.EUR),
        new Fraction(3))
    ]);
    const pair = new CurrencyPair(Currencies.USD, Currencies.EUR);
    assert.ok(table.find(pair).rate.equals(new Fraction(6)));
    assert.ok(table.find(pair.invert()).rate.equals(new Fraction(1, 6)));
    table.add(new ExchangeRate(
      new CurrencyPair(Currencies.CAD, Currencies.USD), new Fraction(1, 4)));
    assert.ok(table.find(pair).rate.equals(new Fraction(12)));
    assert.ok(table.find(pair.invert()).rate.equals(new Fraction(1, 12)));
    table.add(new ExchangeRate(pair, new Fraction(5)));
    assert.ok(table.find(pair).rate.equals(new Fraction(5)));
    assert.ok(table.find(pair.invert()).rate.equals(new Fraction(1, 5)));
  });

  it('shortest_conversion_path', () => {
    const table = new ExchangeRateTable([
      new ExchangeRate(new CurrencyPair(Currencies.USD, Currencies.CAD),
        new Fraction(2)),
      new ExchangeRate(new CurrencyPair(Currencies.CAD, Currencies.EUR),
        new Fraction(3)),
      new ExchangeRate(new CurrencyPair(Currencies.EUR, Currencies.GBP),
        new Fraction(5)),
      new ExchangeRate(new CurrencyPair(Currencies.CAD, Currencies.GBP),
        new Fraction(7))
    ]);
    assert.ok(table.find(new CurrencyPair(Currencies.USD, Currencies.GBP)).
      rate.equals(new Fraction(14)));
  });

  it('round_trip_json', () => {
    const table = new ExchangeRateTable([
      new ExchangeRate(new CurrencyPair(Currencies.USD, Currencies.CAD),
        new Fraction(3, 2)),
      new ExchangeRate(new CurrencyPair(Currencies.CAD, Currencies.EUR),
        new Fraction(4, 3))
    ]);
    const pair = new CurrencyPair(Currencies.USD, Currencies.EUR);
    assert.ok(table.find(pair).rate.equals(new Fraction(2)));
    const jsonValue = table.toJson();
    assert.strictEqual(jsonValue.direct_rates.length, 4);
    assert.deepStrictEqual(jsonValue.direct_rates[0], [
      {base: 840, counter: 124},
      {pair: {base: 840, counter: 124}, rate: {numerator: 3, denominator: 2}}
    ]);
    const restored = ExchangeRateTable.fromJson(jsonValue);
    assert.deepStrictEqual(restored.toJson(), jsonValue);
    assert.ok(restored.find(pair).rate.equals(new Fraction(2)));
    assert.ok(restored.find(pair.invert()).rate.equals(new Fraction(1, 2)));
    restored.add(new ExchangeRate(
      new CurrencyPair(Currencies.CAD, Currencies.EUR), new Fraction(2)));
    assert.ok(restored.find(pair).rate.equals(new Fraction(3)));
    assert.ok(table.find(pair).rate.equals(new Fraction(2)));
  });
});
