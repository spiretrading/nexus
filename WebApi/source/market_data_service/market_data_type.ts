import * as Beam from 'beam';

/** Enumerates the types of market data. */
export enum MarketDataType {

  /** Represents TimeAndSale data. */
  TIME_AND_SALE = 0,

  /** Represents BookQuote data. */
  BOOK_QUOTE,

  /** Represents BboQuote data. */
  BBO_QUOTE,

  /** Represents OrderImbalance data. */
  ORDER_IMBALANCE,

  /** Represents TickerStatus data. */
  TICKER_STATUS
}

export class MarketDataTypeSet extends Beam.EnumSet<MarketDataType> {}
