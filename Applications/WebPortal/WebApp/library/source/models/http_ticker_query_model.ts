import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { TickerQueryModel } from './ticker_query_model';

/** Supplies ticker suggestions through the market data service. */
export class HttpTickerQueryModel extends TickerQueryModel {
  public async submit(query: string): Promise<readonly Nexus.Ticker[]> {
    const prefix = query.trim().toUpperCase();
    if(!prefix) {
      return [];
    }
    const prefixes = new Set([prefix]);
    const separator = prefix.lastIndexOf('.');
    if(separator !== -1 && separator < prefix.length - 1) {
      const venuePrefix = prefix.substring(separator + 1);
      for(const venue of Nexus.venueDatabase) {
        if(venue.displayName.toUpperCase().startsWith(venuePrefix)) {
          prefixes.add(`${prefix.substring(0, separator + 1)}${venue.venue}`);
        }
      }
    }
    const responses = await Promise.all(Array.from(prefixes, prefix =>
      Beam.post('/api/market_data_service/load_ticker_info_from_prefix',
        {prefix})));
    const tickers: Nexus.Ticker[] = [];
    for(const response of responses) {
      for(const info of response) {
        const ticker = Nexus.Ticker.fromJson(info.ticker);
        if(!tickers.some(entry => entry.equals(ticker))) {
          tickers.push(ticker);
        }
      }
    }
    return tickers;
  }
}
