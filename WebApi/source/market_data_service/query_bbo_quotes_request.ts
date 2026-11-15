import * as Beam from 'beam';
import { BboQuote } from '../definitions/bbo_quote';
import { SequencedBboQuote, TickerQuery } from './ticker_query';

/** Queries a ticker's best bid and ask quotes. */
export class QueryBboQuotesRequest implements
    Beam.ServiceRequest<Beam.QueryResult<SequencedBboQuote>> {
  public readonly service = 'Nexus.MarketDataService.QueryBboQuotesService';

  /** The query to submit. */
  public readonly query: TickerQuery;

  /** Constructs a request for a ticker's BBO quotes. */
  constructor(query: TickerQuery) {
    this.query = query;
  }

  public toJson(): any {
    return {query: this.query.toJson()};
  }

  public parseResponse(value: any): Beam.QueryResult<SequencedBboQuote> {
    return new Beam.QueryResult(value.id, value.snapshot.map((entry: any) =>
      Beam.SequencedValue.fromJson<BboQuote>(BboQuote, entry)));
  }
}
