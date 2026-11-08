import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import * as Nexus from '../../source';
import { getAveragePrice, getSide, Position } from
  '../../source/accounting/position';
import { Currency } from '../../source/definitions/currency';
import { Money } from '../../source/definitions/money';
import { Quantity } from '../../source/definitions/quantity';
import { Side } from '../../source/definitions/side';
import { Currencies } from '../../source/definitions/standard_currencies';
import { Ticker } from '../../source/definitions/ticker';

describe('Position', () => {
  it('empty_position', () => {
    const position = new Position();
    assert.ok(position.ticker.equals(Ticker.NONE));
    assert.ok(position.currency.equals(Currency.NONE));
    assert.ok(position.quantity.equals(Quantity.ZERO));
    assert.ok(position.costBasis.equals(Money.ZERO));
    const ticker = Ticker.parse('TST.TSX');
    const identified = new Position(ticker, Currencies.CAD);
    assert.ok(identified.ticker.equals(ticker));
    assert.ok(identified.currency.equals(Currencies.CAD));
    assert.ok(identified.quantity.equals(Quantity.ZERO));
    assert.ok(identified.costBasis.equals(Money.ZERO));
  });

  it('average_price_and_side', () => {
    const ticker = Ticker.parse('TST.TSX');
    const cases: [string, string, string, Side][] = [
      ['100', '1234', '12.34', Side.BID],
      ['-100', '-1234', '12.34', Side.ASK],
      ['2.5', '30.85', '12.34', Side.BID],
      ['-2.5', '-30.85', '12.34', Side.ASK],
      ['0', '10', '0', Side.NONE]
    ];
    for(const [quantity, cost, average, side] of cases) {
      const position = new Position(ticker, Currencies.CAD,
        new Quantity(quantity), new Money(cost));
      assert.ok(getAveragePrice(position).equals(new Money(average)));
      assert.strictEqual(getSide(position), side);
      assert.strictEqual(Nexus.getSide(position), side);
      assert.strictEqual(Nexus.getSide(position.quantity), side);
    }
  });

  it('round_trip_json', () => {
    const position = new Position(Ticker.parse('TST.TSX'), Currencies.CAD,
      new Quantity('-2.5'), new Money('-30.85'));
    const json = {
      ticker: {symbol: 'TST', venue: 'XTSE'},
      currency: 124,
      quantity: -2500000,
      cost_basis: -30850000
    };
    assert.deepStrictEqual(position.toJson(), json);
    const restored = Position.fromJson(json);
    assert.ok(restored.equals(position));
    assert.deepStrictEqual(restored.toJson(), json);
    assert.ok(getAveragePrice(restored).equals(new Money('12.34')));
    assert.strictEqual(getSide(restored), Side.ASK);
  });

  it('equality', () => {
    const position = new Position(Ticker.parse('TST.TSX'), Currencies.CAD,
      new Quantity('10'), new Money('100'));
    const changes: ((value: Position) => void)[] = [
      value => { value.ticker = Ticker.parse('OTHER.TSX'); },
      value => { value.currency = Currencies.USD; },
      value => { value.quantity = new Quantity('11'); },
      value => { value.costBasis = new Money('101'); }
    ];
    for(const change of changes) {
      const other = Position.fromJson(position.toJson());
      assert.ok(position.equals(other));
      change(other);
      assert.ok(!position.equals(other));
    }
  });
});
