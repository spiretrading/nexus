import * as Nexus from 'nexus';
import { QueryModel } from './query_model';

/** Parses qualified tickers and supplies ticker suggestions. */
export abstract class TickerQueryModel extends QueryModel<Nexus.Ticker> {

  /** Parses a ticker with a venue name or code, without checking its listing.
   * @return The ticker, or null if the query is not a qualified ticker.
   */
  public async parse(query: string): Promise<Nexus.Ticker> {
    const ticker = Nexus.Ticker.parse(query.trim().toUpperCase());
    if(!ticker.symbol || /\s/.test(ticker.symbol) ||
        ticker.venue.equals(Nexus.Venue.NONE)) {
      return null;
    }
    return ticker;
  }
}
