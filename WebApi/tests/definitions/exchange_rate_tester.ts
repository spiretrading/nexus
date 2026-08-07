import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import Fraction from 'fraction.js';
import { Currency } from '../../source/definitions/currency';
import { CurrencyPair } from '../../source/definitions/currency_pair';
import { ExchangeRate } from '../../source/definitions/exchange_rate';
import { Money } from '../../source/definitions/money';

describe('ExchangeRate', () => {
  it('construct', () => {
    const pair = new CurrencyPair(new Currency(840), new Currency(978));
    const rate = new Fraction(3, 4);
    const exchangeRate = new ExchangeRate(pair, rate);
    assert.ok(exchangeRate.pair.equals(pair));
    assert.ok(exchangeRate.rate.equals(rate));
  });

  it('none', () => {
    assert.ok(ExchangeRate.NONE.pair.equals(CurrencyPair.NONE));
    assert.ok(ExchangeRate.NONE.rate.equals(new Fraction(1)));
  });

  it('equals', () => {
    const a = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(978)),
      new Fraction(3, 4));
    const b = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(978)),
      new Fraction(3, 4));
    assert.ok(a.equals(b));
  });

  it('not_equal_pair', () => {
    const a = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(978)),
      new Fraction(3, 4));
    const b = new ExchangeRate(
      new CurrencyPair(new Currency(124), new Currency(978)),
      new Fraction(3, 4));
    assert.ok(!a.equals(b));
  });

  it('not_equal_rate', () => {
    const a = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(978)),
      new Fraction(3, 4));
    const b = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(978)),
      new Fraction(5, 4));
    assert.ok(!a.equals(b));
  });

  it('invert', () => {
    const pair = new CurrencyPair(new Currency(826), new Currency(392));
    const rate = new Fraction(2, 5);
    const exchangeRate = new ExchangeRate(pair, rate);
    const inverted = exchangeRate.invert();
    assert.ok(inverted.rate.equals(new Fraction(5, 2)));
    assert.ok(inverted.pair.equals(pair.invert()));
  });

  it('invert_twice', () => {
    const pair = new CurrencyPair(new Currency(36), new Currency(124));
    const rate = new Fraction(7, 9);
    const exchangeRate = new ExchangeRate(pair, rate);
    const back = exchangeRate.invert().invert();
    assert.ok(back.equals(exchangeRate));
  });

  it('convert', () => {
    const rate = new Fraction(3, 2);
    const original = Money.ONE.multiply(100);
    const exchangeRate = new ExchangeRate(
      new CurrencyPair(new Currency(978), new Currency(826)), rate);
    const converted = exchangeRate.convert(original);
    const expected = original.multiply(3).divide(2);
    assert.ok(converted.equals(expected));
  });

  it('convert_then_invert', () => {
    const rate = new Fraction(5, 3);
    const original = Money.ONE.multiply(45);
    const exchangeRate = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(124)), rate);
    const converted = exchangeRate.convert(original);
    const back = exchangeRate.invert().convert(converted);
    assert.ok(back.equals(original));
  });

  it('to_json', () => {
    const exchangeRate = new ExchangeRate(
      new CurrencyPair(new Currency(978), new Currency(840)),
      new Fraction(3, 2));
    const json = exchangeRate.toJson();
    assert.strictEqual(json.pair.base, 978);
    assert.strictEqual(json.pair.counter, 840);
    assert.strictEqual(json.rate.numerator, 3);
    assert.strictEqual(json.rate.denominator, 2);
  });

  it('from_json', () => {
    const json = {
      pair: { base: 978, counter: 840 },
      rate: { numerator: 3, denominator: 2 }
    };
    const exchangeRate = ExchangeRate.fromJson(json);
    assert.ok(exchangeRate.pair.base.equals(new Currency(978)));
    assert.ok(exchangeRate.pair.counter.equals(new Currency(840)));
    assert.ok(exchangeRate.rate.equals(new Fraction(3, 2)));
  });

  it('round_trip_json', () => {
    const original = new ExchangeRate(
      new CurrencyPair(new Currency(840), new Currency(124)),
      new Fraction(7, 4));
    const restored = ExchangeRate.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });
});
