import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Quantity } from '../../source/definitions/quantity';

describe('Quantity', () => {
  it('constants', () => {
    assert.ok(Quantity.ZERO.equals(Quantity.from_representation(0)));
    assert.ok(Quantity.ONE.equals(Quantity.from_representation(1000000)));
  });

  it('construct_from_string', () => {
    const quantity = new Quantity('1.5');
    assert.ok(quantity.equals(
      Quantity.ONE.add(Quantity.from_representation(500000))));
  });

  it('construct_invalid_throws', () => {
    assert.throws(() => new Quantity(''));
  });

  it('to_string_integers', () => {
    assert.strictEqual(Quantity.ZERO.toString(), '0');
    assert.strictEqual(Quantity.ONE.toString(), '1');
    assert.strictEqual(Quantity.ONE.multiply(5).toString(), '5');
    assert.strictEqual(Quantity.ONE.multiply(100).toString(), '100');
  });

  it('to_string_decimals', () => {
    assert.strictEqual(Quantity.ONE.divide(10).toString(), '0.1');
    assert.strictEqual(Quantity.ONE.divide(100).toString(), '0.01');
    assert.strictEqual(Quantity.ONE.divide(1000).toString(), '0.001');
    assert.strictEqual(Quantity.ONE.divide(10000).toString(), '0.0001');
    assert.strictEqual(Quantity.ONE.divide(100000).toString(), '0.00001');
    assert.strictEqual(Quantity.ONE.divide(1000000).toString(), '0.000001');
  });

  it('to_string_negative', () => {
    assert.strictEqual(Quantity.ONE.multiply(-1).toString(), '-1');
    assert.strictEqual(Quantity.ONE.multiply(-1).subtract(
      Quantity.from_representation(500000)).toString(), '-1.5');
  });

  it('to_string_trims_trailing_zeros', () => {
    assert.strictEqual(
      Quantity.from_representation(1100000).toString(), '1.1');
    assert.strictEqual(
      Quantity.from_representation(1000100).toString(), '1.0001');
  });

  it('parse', () => {
    assert.ok(Quantity.parse('0').equals(Quantity.ZERO));
    assert.ok(Quantity.parse('1').equals(Quantity.ONE));
    assert.ok(Quantity.parse('1.1').equals(
      Quantity.from_representation(1100000)));
  });

  it('parse_with_sign', () => {
    assert.ok(Quantity.parse('+1').equals(Quantity.ONE));
    assert.ok(Quantity.parse('-1').equals(Quantity.ONE.multiply(-1)));
    assert.ok(Quantity.parse('-0').equals(Quantity.ZERO));
    assert.ok(Quantity.parse('+0').equals(Quantity.ZERO));
  });

  it('parse_empty_returns_null', () => {
    assert.strictEqual(Quantity.parse(''), null);
  });

  it('parse_invalid_returns_null', () => {
    assert.strictEqual(Quantity.parse('1a.1'), null);
  });

  it('add', () => {
    const result = Quantity.ONE.add(Quantity.ONE);
    assert.ok(result.equals(Quantity.ONE.multiply(2)));
  });

  it('subtract', () => {
    const result = Quantity.ONE.multiply(3).subtract(Quantity.ONE);
    assert.ok(result.equals(Quantity.ONE.multiply(2)));
  });

  it('multiply', () => {
    const result = Quantity.ONE.multiply(5);
    assert.ok(result.equals(Quantity.from_representation(5000000)));
  });

  it('divide', () => {
    const result = Quantity.ONE.divide(4);
    assert.ok(result.equals(Quantity.from_representation(250000)));
  });

  it('compare', () => {
    assert.ok(Quantity.ONE.compare(Quantity.ZERO) > 0);
    assert.ok(Quantity.ZERO.compare(Quantity.ONE) < 0);
    assert.strictEqual(Quantity.ONE.compare(Quantity.ONE), 0);
  });

  it('equals', () => {
    assert.ok(Quantity.ONE.equals(Quantity.ONE));
    assert.ok(!Quantity.ONE.equals(Quantity.ZERO));
  });

  it('to_json', () => {
    assert.strictEqual(Quantity.ONE.toJson(), 1000000);
    assert.strictEqual(Quantity.ZERO.toJson(), 0);
  });

  it('from_json', () => {
    const quantity = Quantity.fromJson(1000000);
    assert.ok(quantity.equals(Quantity.ONE));
  });

  it('round_trip_json', () => {
    const original = Quantity.from_representation(12345678);
    const restored = Quantity.fromJson(original.toJson());
    assert.ok(original.equals(restored));
  });
});
