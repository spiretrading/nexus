import * as Beam from 'beam';
import { BboQuote } from '../definitions/bbo_quote';
import { Ticker } from '../definitions/ticker';

/** Defines a query for a ticker's market data. */
export type TickerQuery = Beam.BasicQuery<Ticker>;

/** A BBO with its sequence number. */
export type SequencedBboQuote = Beam.SequencedValue<BboQuote>;

/** A BBO with its ticker. */
export type TickerBboQuote = Beam.IndexedValue<BboQuote, Ticker>;

/** A BBO with its ticker and sequence number. */
export type SequencedTickerBboQuote = Beam.SequencedValue<TickerBboQuote>;
