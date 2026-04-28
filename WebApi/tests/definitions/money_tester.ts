import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Money } from '../../source/definitions/money';

describe('Money', () => {
  it('constants', () => {
    assert.ok(Money.ZERO.equals(Money.from_representation(0)));
    assert.ok(Money.EPSILON.equals(Money.from_representation(1)));
    assert.ok(Money.BIP.equals(Money.from_representation(100)));
    assert.ok(Money.CENT.equals(Money.from_representation(10000)));
    assert.ok(Money.ONE.equals(Money.from_representation(1000000)));
  });

  it('construct_from_string', () => {
    const money = new Money('1.50');
    assert.ok(money.equals(Money.ONE.add(Money.CENT.multiply(50))));
  });

  it('construct_invalid_throws', () => {
    assert.throws(() => new Money(''));
  });

  it('to_string', () => {
    assert.strictEqual(Money.ZERO.toString(), '0.00');
    assert.strictEqual(Money.ONE.toString(), '1.00');
    assert.strictEqual(Money.ONE.divide(10).toString(), '0.10');
    assert.strictEqual(Money.ONE.divide(100).toString(), '0.01');
    assert.strictEqual(Money.ONE.divide(1000).toString(), '0.001');
    assert.strictEqual(Money.ONE.divide(10000).toString(), '0.0001');
    assert.strictEqual(Money.ONE.divide(100000).toString(), '0.00001');
    assert.strictEqual(Money.ONE.divide(1000000).toString(), '0.000001');
    assert.strictEqual(
      Money.CENT.multiply(13).add(Money.CENT.divide(2)).toString(), '0.135');
    assert.strictEqual(
      Money.CENT.multiply(3).add(Money.CENT.divide(2)).toString(), '0.035');
  });

  it('to_string_negative', () => {
    assert.strictEqual(Money.ONE.multiply(-1).toString(), '-1.00');
    assert.strictEqual(Money.CENT.multiply(-1).toString(), '-0.01');
  });

  it('parse', () => {
    assert.ok(Money.parse('0').equals(Money.ZERO));
    assert.ok(Money.parse('0.01').equals(Money.CENT));
    assert.ok(Money.parse('0.02').equals(Money.CENT.multiply(2)));
    assert.ok(Money.parse('0.10').equals(Money.CENT.multiply(10)));
    assert.ok(Money.parse('0.11').equals(Money.CENT.multiply(11)));
    assert.ok(Money.parse('0.12').equals(Money.CENT.multiply(12)));
    assert.ok(Money.parse('1.00').equals(Money.ONE));
    assert.ok(Money.parse('1.01').equals(Money.ONE.add(Money.CENT)));
    assert.ok(Money.parse('1.12').equals(
      Money.ONE.add(Money.CENT.multiply(12))));
    assert.ok(Money.parse('2.01').equals(
      Money.ONE.multiply(2).add(Money.CENT)));
  });

  it('parse_with_sign', () => {
    assert.ok(Money.parse('+0.01').equals(Money.CENT));
    assert.ok(Money.parse('+1.00').equals(Money.ONE));
    assert.ok(Money.parse('-0.01').equals(Money.CENT.multiply(-1)));
    assert.ok(Money.parse('-1.00').equals(Money.ONE.multiply(-1)));
    assert.ok(Money.parse('-1.01').equals(
      Money.ONE.add(Money.CENT).multiply(-1)));
  });

  it('parse_zero_variants', () => {
    assert.ok(Money.parse('-0').equals(Money.ZERO));
    assert.ok(Money.parse('+0').equals(Money.ZERO));
  });

  it('parse_empty_returns_null', () => {
    assert.strictEqual(Money.parse(''), null);
  });

  it('parse_invalid_returns_null', () => {
    assert.strictEqual(Money.parse('abc'), null);
  });

  it('parse_truncates_extra_decimals', () => {
    assert.ok(Money.parse('0.00000100').equals(Money.ONE.divide(1000000)));
  });

  it('add', () => {
    const result = Money.ONE.add(Money.CENT);
    assert.ok(result.equals(Money.from_representation(1010000)));
  });

  it('subtract', () => {
    const result = Money.ONE.subtract(Money.CENT);
    assert.ok(result.equals(Money.from_representation(990000)));
  });

  it('multiply', () => {
    const result = Money.ONE.multiply(5);
    assert.ok(result.equals(Money.from_representation(5000000)));
  });

  it('divide', () => {
    const result = Money.ONE.divide(4);
    assert.ok(result.equals(Money.from_representation(250000)));
  });

  it('compare', () => {
    assert.ok(Money.ONE.compare(Money.CENT) > 0);
    assert.ok(Money.CENT.compare(Money.ONE) < 0);
    assert.strictEqual(Money.ONE.compare(Money.ONE), 0);
  });

  it('equals', () => {
    assert.ok(Money.ONE.equals(Money.ONE));
    assert.ok(!Money.ONE.equals(Money.CENT));
  });

  it('to_json', () => {
    assert.strictEqual(Money.ONE.toJson(), 1000000);
    assert.strictEqual(Money.CENT.toJson(), 10000);
  });

  it('from_json', () => {
    const money = Money.fromJson(1000000);
    assert.ok(money.equals(Money.ONE));
  });

  it('round_trip_json', () => {
    const original = Money.ONE.multiply(42).add(Money.CENT.multiply(99));
    const restored = Money.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });
});
