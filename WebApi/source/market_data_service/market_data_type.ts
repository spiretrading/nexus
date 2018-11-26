import * as Beam from 'beam';

/** Enumerates the types of market data. */
export enum MarketDataType {

  /** Represents no data. */
  NONE = 0,

  /** Represents TimeAndSale data. */
  TIME_AND_SALE,

  /** Represents BookQuote data. */
  BOOK_QUOTE,

  /** Represents MarketQuote data. */
  MARKET_QUOTE,

  /** Represents BboQuote data. */
  BBO_QUOTE,

  /** Represents OrderImbalance data. */
  ORDER_IMBALANCE
}

export class MarketDataTypeSet extends Beam.EnumSet<MarketDataType> {}
