import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { Currencies, Currency, Money, Valuation } from '../../source';

describe('Valuation', () => {
  it('empty_valuation', () => {
    const empty = new Valuation();
    assert.ok(empty.currency.equals(Currency.NONE));
    assert.strictEqual(empty.askValue, null);
    assert.strictEqual(empty.bidValue, null);
    const valuation = new Valuation(Currencies.CAD);
    assert.ok(valuation.currency.equals(Currencies.CAD));
    assert.strictEqual(valuation.askValue, null);
    assert.strictEqual(valuation.bidValue, null);
  });

  it('round_trip_json', () => {
    const cases: [Money, Money, any, any][] = [
      [null, null, {is_initialized: false}, {is_initialized: false}],
      [Money.ZERO, null,
        {is_initialized: true, value: 0}, {is_initialized: false}],
      [null, Money.ZERO,
        {is_initialized: false}, {is_initialized: true, value: 0}],
      [new Money('-12.345678'), new Money('12.345679'),
        {is_initialized: true, value: -12345678},
        {is_initialized: true, value: 12345679}]
    ];
    for(const [ask, bid, askJson, bidJson] of cases) {
      const valuation = new Valuation(Currencies.CAD);
      valuation.askValue = ask;
      valuation.bidValue = bid;
      const json = {currency: 124, ask_value: askJson, bid_value: bidJson};
      assert.deepStrictEqual(valuation.toJson(), json);
      const restored = Valuation.fromJson(json);
      assert.ok(restored.currency.equals(Currencies.CAD));
      if(ask) {
        assert.ok(restored.askValue.equals(ask));
      } else {
        assert.strictEqual(restored.askValue, null);
      }
      if(bid) {
        assert.ok(restored.bidValue.equals(bid));
      } else {
        assert.strictEqual(restored.bidValue, null);
      }
      assert.deepStrictEqual(restored.toJson(), json);
    }
  });
});
