import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { MarketDataType, MarketDataTypeSet } from
  '../../source/market_data_service/market_data_type';

describe('MarketDataTypeSet', () => {
  it('consolidated_level_one', () => {
    const types = MarketDataTypeSet.fromJson<MarketDataType>(5);
    assert.ok(types.test(MarketDataType.TIME_AND_SALE));
    assert.ok(types.test(MarketDataType.BBO_QUOTE));
    assert.ok(!types.test(MarketDataType.BOOK_QUOTE));
    assert.ok(!types.test(MarketDataType.ORDER_IMBALANCE));
    assert.ok(!types.test(MarketDataType.TICKER_STATUS));
  });

  it('serialized_flags', () => {
    const flags = [
      [MarketDataType.TIME_AND_SALE, 1],
      [MarketDataType.BOOK_QUOTE, 2],
      [MarketDataType.BBO_QUOTE, 4],
      [MarketDataType.ORDER_IMBALANCE, 8],
      [MarketDataType.TICKER_STATUS, 16]
    ];
    for(const [type, mask] of flags) {
      const types = MarketDataTypeSet.fromJson<MarketDataType>(mask);
      for(const [other] of flags) {
        assert.strictEqual(types.test(other), other === type);
      }
      const encoded = new MarketDataTypeSet();
      encoded.set(type);
      assert.strictEqual(encoded.toJson(), mask);
    }
  });
});
