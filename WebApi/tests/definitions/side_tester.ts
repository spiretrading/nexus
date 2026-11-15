import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { Quantity } from '../../source/definitions/quantity';
import { getDirection, getOpposite, getSide, pick, Side, toChar } from
  '../../source/definitions/side';

describe('Side', () => {
  it('pick', () => {
    const ask = {price: 10};
    const bid = {price: 20};
    assert.strictEqual(pick(Side.ASK, ask, bid), ask);
    assert.strictEqual(pick(Side.BID, ask, bid), bid);
    assert.strictEqual(pick(Side.NONE, ask, bid), bid);
  });

  it('get_direction', () => {
    assert.strictEqual(getDirection(Side.ASK), -1);
    assert.strictEqual(getDirection(Side.BID), 1);
    assert.strictEqual(getDirection(Side.NONE), 0);
  });

  it('get_side', () => {
    assert.strictEqual(getSide(new Quantity('10')), Side.BID);
    assert.strictEqual(getSide(new Quantity('-10')), Side.ASK);
    assert.strictEqual(getSide(new Quantity('0.000001')), Side.BID);
    assert.strictEqual(getSide(new Quantity('-0.000001')), Side.ASK);
    assert.strictEqual(getSide(Quantity.ZERO), Side.NONE);
  });

  it('get_opposite', () => {
    assert.strictEqual(getOpposite(Side.ASK), Side.BID);
    assert.strictEqual(getOpposite(Side.BID), Side.ASK);
    assert.strictEqual(getOpposite(Side.NONE), Side.NONE);
  });

  it('to_char', () => {
    assert.strictEqual(toChar(Side.ASK), 'A');
    assert.strictEqual(toChar(Side.BID), 'B');
    assert.strictEqual(toChar(Side.NONE), '?');
  });
});
