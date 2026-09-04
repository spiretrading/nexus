import * as Beam from 'beam';

/**
 * The types that Nexus registers in addition to the types Beam registers,
 * named as the services expect them.
 */
export namespace QueryType {

  /** An account modification request. */
  export const ACCOUNT_MODIFICATION_REQUEST: Beam.QueryType =
    'Nexus.AccountModificationRequest';

  /** A best bid and offer quote. */
  export const BBO_QUOTE: Beam.QueryType = 'Nexus.BboQuote';

  /** A quote within a book. */
  export const BOOK_QUOTE: Beam.QueryType = 'Nexus.BookQuote';

  /** A monetary value. */
  export const MONEY: Beam.QueryType = 'Nexus.Money';

  /** The fields used to submit an order. */
  export const ORDER_FIELDS: Beam.QueryType = 'Nexus.OrderFields';

  /** An imbalance in an auction. */
  export const ORDER_IMBALANCE: Beam.QueryType = 'Nexus.OrderImbalance';

  /** The details of a submitted order. */
  export const ORDER_INFO: Beam.QueryType = 'Nexus.OrderInfo';

  /** A quantity of a ticker. */
  export const QUANTITY: Beam.QueryType = 'Nexus.Quantity';

  /** A price and size on one side of a book. */
  export const QUOTE: Beam.QueryType = 'Nexus.Quote';

  /** The side of a transaction. */
  export const SIDE: Beam.QueryType = 'Nexus.Side';

  /** A ticker symbol. */
  export const TICKER: Beam.QueryType = 'Nexus.Ticker';

  /** The details of a ticker. */
  export const TICKER_INFO: Beam.QueryType = 'Nexus.TickerInfo';

  /** The trading status of a ticker. */
  export const TICKER_STATUS: Beam.QueryType = 'Nexus.TickerStatus';

  /** A completed transaction. */
  export const TIME_AND_SALE: Beam.QueryType = 'Nexus.TimeAndSale';

  /** A venue that a ticker trades on. */
  export const VENUE: Beam.QueryType = 'Nexus.Venue';
}
