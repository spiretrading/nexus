import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { Inventory, isEmpty } from '../../source/accounting/inventory';
import { Position } from '../../source/accounting/position';
import { Money } from '../../source/definitions/money';
import { Quantity } from '../../source/definitions/quantity';
import { Currencies } from '../../source/definitions/standard_currencies';
import { Ticker } from '../../source/definitions/ticker';

describe('Inventory', () => {
  it('empty_inventory', () => {
    const ticker = Ticker.parse('TST.TSX');
    const inventory = new Inventory(ticker, Currencies.CAD);
    assert.ok(inventory.position.ticker.equals(ticker));
    assert.ok(inventory.position.currency.equals(Currencies.CAD));
    assert.ok(isEmpty(inventory));
    assert.ok(isEmpty(new Inventory()));
    const changes: ((value: Inventory) => void)[] = [
      value => { value.position.quantity = Quantity.ONE; },
      value => { value.position.costBasis = Money.ONE; },
      value => { value.grossProfitAndLoss = Money.ONE; },
      value => { value.fees = Money.CENT; },
      value => { value.volume = Quantity.ONE; },
      value => { value.transactionCount = 1; }
    ];
    for(const change of changes) {
      const other = new Inventory(ticker, Currencies.CAD);
      assert.ok(other.equals(inventory));
      change(other);
      assert.ok(!isEmpty(other));
      assert.ok(!other.equals(inventory));
    }
  });

  it('round_trip_json', () => {
    const position = new Position(Ticker.parse('TST.TSX'), Currencies.CAD,
      new Quantity('-2.5'), new Money('-30.85'));
    const inventory = new Inventory(position, new Money('-12.34'),
      new Money('0.25'), new Quantity('10.5'), 3);
    const json = {
      position: {
        ticker: {symbol: 'TST', venue: 'XTSE'},
        currency: 124,
        quantity: -2500000,
        cost_basis: -30850000
      },
      gross_profit_and_loss: -12340000,
      fees: 250000,
      volume: 10500000,
      transaction_count: 3
    };
    assert.deepStrictEqual(inventory.toJson(), json);
    const restored = Inventory.fromJson(json);
    assert.ok(restored.equals(inventory));
    assert.deepStrictEqual(restored.toJson(), json);
    assert.ok(!isEmpty(restored));
    restored.position.quantity = Quantity.ZERO;
    assert.ok(inventory.position.quantity.equals(new Quantity('-2.5')));
  });
});
