import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import * as Beam from 'beam';
import { Money } from '../../source/definitions/money';
import { Quantity } from '../../source/definitions/quantity';
import { Ticker } from '../../source/definitions/ticker';
import { Venue } from '../../source/definitions/venue';
import { QueryType } from '../../source/queries/query_type';
import { MoneyValue, QuantityValue, TickerValue, VenueValue } from
  '../../source/queries/standard_values';

describe('StandardValues', () => {
  const TICKER = new Ticker('ABX', new Venue('XTSE'));

  it('type', () => {
    assert.strictEqual(new QuantityValue(new Quantity('100')).type,
      QueryType.QUANTITY);
    assert.strictEqual(new MoneyValue(Money.ONE).type, QueryType.MONEY);
    assert.strictEqual(new TickerValue(TICKER).type, QueryType.TICKER);
    assert.strictEqual(
      new VenueValue(new Venue('XTSE')).type, QueryType.VENUE);
  });

  it('sends_the_registered_type_names', () => {
    assert.strictEqual(new QuantityValue(new Quantity('1')).toJson().__type,
      'Nexus.Queries.QuantityValue');
    assert.strictEqual(new MoneyValue(Money.ONE).toJson().__type,
      'Nexus.Queries.MoneyValue');
    assert.strictEqual(new TickerValue(TICKER).toJson().__type,
      'Nexus.Queries.TickerValue');
    assert.strictEqual(new VenueValue(new Venue('XTSE')).toJson().__type,
      'Nexus.Queries.VenueValue');
  });

  it('equals', () => {
    assert.ok(new MoneyValue(Money.ONE).equals(new MoneyValue(Money.ONE)));
    assert.ok(!new MoneyValue(Money.ONE).equals(new MoneyValue(Money.CENT)));
    assert.ok(!new MoneyValue(Money.ONE).equals(
      new QuantityValue(new Quantity('1'))));
  });

  it('round_trip', () => {
    const values: Beam.Value[] = [new QuantityValue(new Quantity('100')),
      new MoneyValue(Money.ONE.multiply(3)), new TickerValue(TICKER),
      new VenueValue(new Venue('XTSE'))];
    for(const value of values) {
      const json = JSON.parse(JSON.stringify(value.toJson()));
      const restored = Beam.Value.fromJson(json);
      assert.strictEqual(restored.type, value.type);
      assert.ok(restored.equals(value), `${value} did not round trip.`);
    }
  });

  it('round_trips_inside_a_constant_expression', () => {
    const expression = new Beam.ConstantExpression(new MoneyValue(Money.ONE));
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.value.value.__type, 'Nexus.Queries.MoneyValue');
    assert.ok(Beam.Expression.fromJson(json).equals(expression));
  });
});
