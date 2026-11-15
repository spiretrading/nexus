import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { Inventory, Money, Position, Quantity, RiskInventoryEntry,
  RiskPortfolioKey, Ticker, Venue } from '../../source';
import { Currencies } from '../../source/definitions/standard_currencies';

describe('RiskPortfolioTypes', () => {
  it('key_equality_and_hashing', () => {
    const ticker = Ticker.parse('TST.TSX');
    const key = new RiskPortfolioKey(
      Beam.DirectoryEntry.makeAccount(123, 'trader'), ticker);
    const renamed = new RiskPortfolioKey(
      Beam.DirectoryEntry.makeAccount(123, 'renamed'),
      new Ticker('TST', new Venue('XTSE')));
    assert.ok(key.equals(renamed));
    assert.strictEqual(key.hash(), renamed.hash());
    const otherAccount = new RiskPortfolioKey(
      Beam.DirectoryEntry.makeAccount(456, 'trader'), ticker);
    const otherTicker = new RiskPortfolioKey(key.account,
      new Ticker('OTHER', ticker.venue));
    const otherVenue = new RiskPortfolioKey(key.account,
      new Ticker('TST', new Venue('XNYS')));
    const entries = new Beam.Map<RiskPortfolioKey, number>();
    entries.set(key, 1);
    entries.set(otherAccount, 2);
    entries.set(otherTicker, 3);
    entries.set(otherVenue, 4);
    assert.strictEqual(entries.get(renamed), 1);
    assert.strictEqual(entries.get(otherAccount), 2);
    assert.strictEqual(entries.get(otherTicker), 3);
    assert.strictEqual(entries.get(otherVenue), 4);
    assert.ok(!key.equals(otherAccount));
    assert.ok(!key.equals(otherTicker));
    assert.ok(!key.equals(otherVenue));
  });

  it('entry_round_trip_json', () => {
    const ticker = Ticker.parse('TST.TSX');
    const key = new RiskPortfolioKey(
      Beam.DirectoryEntry.makeAccount(123, 'trader'), ticker);
    const inventory = new Inventory(new Position(ticker, Currencies.CAD,
      new Quantity('-2.5'), new Money('-30.85')), new Money('-12.34'),
      new Money('0.25'), new Quantity('10.5'), 3);
    const entry = new RiskInventoryEntry(key, inventory);
    const json = {
      key: {
        account: {type: 0, id: 123, name: 'trader'},
        ticker: {symbol: 'TST', venue: 'XTSE'}
      },
      value: {
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
      }
    };
    assert.deepStrictEqual(entry.toJson(), json);
    const restored = RiskInventoryEntry.fromJson(json);
    assert.ok(restored.equals(entry));
    assert.strictEqual(restored.key.hash(), key.hash());
    assert.deepStrictEqual(restored.toJson(), json);
  });

  it('entry_equality', () => {
    const ticker = Ticker.parse('TST.TSX');
    const key = new RiskPortfolioKey(
      Beam.DirectoryEntry.makeAccount(123, 'trader'), ticker);
    const entry = new RiskInventoryEntry(key,
      new Inventory(ticker, Currencies.CAD));
    const otherKey = RiskInventoryEntry.fromJson(entry.toJson());
    otherKey.key.account = Beam.DirectoryEntry.makeAccount(456, 'other');
    assert.ok(!entry.equals(otherKey));
    const otherValue = RiskInventoryEntry.fromJson(entry.toJson());
    otherValue.value.position.quantity = Quantity.ONE;
    assert.ok(!entry.equals(otherValue));
    assert.ok(entry.value.position.quantity.equals(Quantity.ZERO));
  });

  it('empty_entry', () => {
    const entry = new RiskInventoryEntry();
    assert.ok(entry.key.account.equals(Beam.DirectoryEntry.INVALID));
    assert.ok(entry.key.ticker.equals(Ticker.NONE));
    assert.ok(entry.value.equals(new Inventory()));
    const restored = RiskInventoryEntry.fromJson(entry.toJson());
    assert.ok(restored.equals(entry));
  });
});
