import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { listingComparator, makeAsk, makeBid, Money, offerComparator, Quantity,
  Quote, Side } from '../../source';

describe('Quote', () => {
  it('empty_quote', () => {
    const quote = new Quote();
    assert.ok(quote.price.equals(Money.ZERO));
    assert.ok(quote.size.equals(Quantity.ZERO));
    assert.strictEqual(quote.side, Side.NONE);
  });

  it('make_ask_and_bid', () => {
    const price = new Money('12.345678');
    const size = new Quantity('2.5');
    assert.ok(makeAsk(price, size).equals(new Quote(price, size, Side.ASK)));
    assert.ok(makeBid(price, size).equals(new Quote(price, size, Side.BID)));
  });

  it('comparators', () => {
    const low = new Money('-0.000001');
    const high = new Money('0.000001');
    const cases: [Side, Money, Money, number][] = [
      [Side.ASK, low, high, -1],
      [Side.ASK, high, low, 1],
      [Side.ASK, low, low, 0],
      [Side.BID, low, high, 1],
      [Side.BID, high, low, -1],
      [Side.BID, high, high, 0],
      [Side.NONE, low, high, 1],
      [Side.NONE, high, low, -1],
      [Side.NONE, low, low, 0]
    ];
    for(const [side, lhs, rhs, expected] of cases) {
      assert.strictEqual(offerComparator(side, lhs, rhs), expected);
      assert.strictEqual(listingComparator(
        new Quote(lhs, Quantity.ZERO, side),
        new Quote(rhs, new Quantity('100'), side)), expected < 0);
    }
  });

  it('round_trip_json', () => {
    const quote = makeAsk(new Money('12.345678'), new Quantity('2.500001'));
    const json = {price: 12345678, size: 2500001, side: 0};
    assert.deepStrictEqual(quote.toJson(), json);
    const restored = Quote.fromJson(json);
    assert.ok(restored.equals(quote));
    assert.deepStrictEqual(restored.toJson(), json);
  });

  it('equality', () => {
    const quote = makeBid(new Money('10'), new Quantity('100'));
    const changes: ((value: Quote) => void)[] = [
      value => { value.price = new Money('11'); },
      value => { value.size = new Quantity('101'); },
      value => { value.side = Side.ASK; }
    ];
    for(const change of changes) {
      const other = Quote.fromJson(quote.toJson());
      assert.ok(quote.equals(other));
      change(other);
      assert.ok(!quote.equals(other));
    }
  });
});
