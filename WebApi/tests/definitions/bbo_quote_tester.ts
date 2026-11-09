import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import { BboQuote, makeAsk, makeBid, Money, Quantity, Side } from
  '../../source';

describe('BboQuote', () => {
  it('empty_quote', () => {
    const quote = new BboQuote();
    assert.ok(quote.bid.equals(makeBid(Money.ZERO, Quantity.ZERO)));
    assert.ok(quote.ask.equals(makeAsk(Money.ZERO, Quantity.ZERO)));
    assert.ok(quote.timestamp.equals(Beam.DateTime.NOT_A_DATE_TIME));
    assert.ok(BboQuote.fromJson(quote.toJson()).equals(quote));
    quote.bid.side = Side.NONE;
    assert.strictEqual(quote.ask.side, Side.ASK);
    assert.strictEqual(new BboQuote().bid.side, Side.BID);
  });

  it('round_trip_json', () => {
    const quote = new BboQuote(
      makeBid(new Money('12.345678'), new Quantity('2.500001')),
      makeAsk(new Money('12.345679'), new Quantity('3.75')),
      Beam.DateTime.fromJson('20261108T153042.125'));
    const json = {
      bid: {price: 12345678, size: 2500001, side: 1},
      ask: {price: 12345679, size: 3750000, side: 0},
      timestamp: '20261108T153042.125'
    };
    assert.deepStrictEqual(quote.toJson(), json);
    const restored = BboQuote.fromJson(json);
    assert.ok(restored.equals(quote));
    assert.deepStrictEqual(restored.toJson(), json);
  });

  it('fractional_timestamp', () => {
    const timestamp = Beam.DateTime.fromJson('20261108T153042.123456');
    const quote = BboQuote.fromJson({
      bid: {price: 10000000, size: 100000000, side: 1},
      ask: {price: 11000000, size: 200000000, side: 0},
      timestamp: '20261108T153042.123456'
    });
    assert.ok(quote.timestamp.equals(timestamp));
    assert.ok(BboQuote.fromJson(quote.toJson()).equals(quote));
  });

  it('equality', () => {
    const quote = new BboQuote(
      makeBid(new Money('10'), new Quantity('100')),
      makeAsk(new Money('11'), new Quantity('200')),
      Beam.DateTime.fromJson('20261108T153000'));
    const changes: ((value: BboQuote) => void)[] = [
      value => { value.bid.price = new Money('9'); },
      value => { value.ask.size = new Quantity('201'); },
      value => { value.timestamp = Beam.DateTime.NOT_A_DATE_TIME; }
    ];
    for(const change of changes) {
      const other = BboQuote.fromJson(quote.toJson());
      assert.ok(quote.equals(other));
      change(other);
      assert.ok(!quote.equals(other));
    }
  });
});
