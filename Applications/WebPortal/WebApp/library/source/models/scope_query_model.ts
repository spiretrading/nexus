import * as Nexus from 'nexus';
import { LocalQueryModel } from './local_query_model';
import { QueryModel } from './query_model';
import { getScopeLabel, isScopeEqual } from './scope';

/** Queries countries, venues, global scope, and a supplied ticker model. */
export class ScopeQueryModel extends QueryModel<Nexus.Scope> {
  constructor(tickers: QueryModel<Nexus.Ticker>) {
    super();
    this.tickers = tickers;
    this.local = new LocalQueryModel(getScopeLabel, isScopeEqual);
    this.local.add('*', Nexus.Scope.GLOBAL);
    this.local.add('Global', Nexus.Scope.GLOBAL);
    for(const country of Nexus.countryDatabase) {
      const scope = new Nexus.Scope(country.code);
      this.local.add(country.twoLetterCode, scope);
      this.local.add(country.threeLetterCode, scope);
      this.local.add(country.name, scope);
    }
    for(const venue of Nexus.venueDatabase) {
      const scope = new Nexus.Scope(venue.venue);
      this.local.add(venue.displayName, scope);
      this.local.add(venue.venue.toString(), scope);
    }
  }

  public async parse(query: string): Promise<Nexus.Scope> {
    const local = await this.local.parse(query.trim());
    if(local) {
      return local.clone();
    }
    const ticker = await this.tickers.parse(query);
    if(ticker === null) {
      return null;
    }
    return new Nexus.Scope(ticker);
  }

  public async submit(query: string): Promise<readonly Nexus.Scope[]> {
    if(!query.trim()) {
      return [];
    }
    const local = await this.local.submit(query.trim());
    if(query.trim() === '*') {
      return local;
    }
    try {
      const tickers = await this.tickers.submit(query);
      return [...local, ...tickers.map(ticker => new Nexus.Scope(ticker))];
    } catch(error) {
      if(local.length === 0) {
        throw error;
      }
      return local;
    }
  }

  private tickers: QueryModel<Nexus.Ticker>;
  private local: LocalQueryModel<Nexus.Scope>;
}
