import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { Currencies, getRealizedProfitAndLoss, getTotalProfitAndLoss,
  getUnrealizedProfitAndLoss, Inventory, Money, Quantity, Ticker,
  Valuation } from '../../source';

function makeInventory(quantity: string, cost: string): Inventory {
  const inventory = new Inventory(Ticker.parse('TST.TSX'), Currencies.CAD);
  inventory.position.quantity = new Quantity(quantity);
  inventory.position.costBasis = new Money(cost);
  return inventory;
}

describe('Portfolio', () => {
  it('realized_profit_and_loss', () => {
    const cases = [
      ['100', '10', '90'],
      ['-100', '10', '-110'],
      ['0', '0', '0'],
      ['12.345678', '-0.000001', '12.345679']
    ];
    for(const [gross, fees, expected] of cases) {
      const inventory = makeInventory('0', '0');
      inventory.grossProfitAndLoss = new Money(gross);
      inventory.fees = new Money(fees);
      assert.ok(getRealizedProfitAndLoss(inventory).equals(
        new Money(expected)));
    }
  });

  it('unrealized_and_total_profit_and_loss', () => {
    const cases = [
      ['100', '1000', '12', '13', '200', '245'],
      ['100', '1500', '12', '13', '-300', '-255'],
      ['-100', '-1500', '12', '13', '200', '245'],
      ['-100', '-1000', '12', '13', '-300', '-255'],
      ['2.5', '25', '12.5', '13', '6.25', '51.25'],
      ['-2.5', '-35', '12', '13.5', '1.25', '46.25'],
      ['0.000001', '0.00001', '12', '13', '0.000002', '45.000002'],
      ['1', '-5', '-2', '-1', '3', '48'],
      ['100', '1000', '10', '11', '0', '45'],
      ['100', '1000', '0', '11', '-1000', '-955'],
      ['-100', '-1000', '10', '0', '1000', '1045']
    ];
    for(const [quantity, cost, bid, ask, unrealized, total] of cases) {
      const inventory = makeInventory(quantity, cost);
      inventory.grossProfitAndLoss = new Money('50');
      inventory.fees = new Money('5');
      const valuation = new Valuation(Currencies.CAD);
      valuation.bidValue = new Money(bid);
      valuation.askValue = new Money(ask);
      assert.ok(getUnrealizedProfitAndLoss(inventory, valuation).equals(
        new Money(unrealized)));
      assert.ok(getTotalProfitAndLoss(inventory, valuation).equals(
        new Money(total)));
    }
  });

  it('missing_prices', () => {
    const inventory = makeInventory('100', '1000');
    inventory.grossProfitAndLoss = new Money('50');
    inventory.fees = new Money('5');
    const valuation = new Valuation(Currencies.CAD);
    valuation.askValue = new Money('12');
    assert.strictEqual(getUnrealizedProfitAndLoss(inventory, valuation), null);
    assert.strictEqual(getTotalProfitAndLoss(inventory, valuation), null);
    valuation.bidValue = new Money('12');
    valuation.askValue = null;
    assert.ok(getUnrealizedProfitAndLoss(inventory, valuation).equals(
      new Money('200')));
    inventory.position.quantity = new Quantity('-100');
    inventory.position.costBasis = new Money('-1000');
    assert.strictEqual(getUnrealizedProfitAndLoss(inventory, valuation), null);
    assert.strictEqual(getTotalProfitAndLoss(inventory, valuation), null);
    valuation.askValue = new Money('9');
    valuation.bidValue = null;
    assert.ok(getUnrealizedProfitAndLoss(inventory, valuation).equals(
      new Money('100')));
    assert.ok(getTotalProfitAndLoss(inventory, valuation).equals(
      new Money('145')));
  });

  it('flat_position', () => {
    const inventory = makeInventory('0', '10');
    inventory.grossProfitAndLoss = new Money('50');
    inventory.fees = new Money('5');
    const valuation = new Valuation(Currencies.CAD);
    assert.ok(getUnrealizedProfitAndLoss(inventory, valuation).equals(
      Money.ZERO));
    assert.ok(getTotalProfitAndLoss(inventory, valuation).equals(
      new Money('45')));
  });
});
