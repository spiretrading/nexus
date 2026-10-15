import * as Nexus from 'nexus';
import { LocalQueryModel } from './local_query_model';
import { TickerQueryModel } from './ticker_query_model';

/** Supplies local ticker suggestions by symbol and venue prefix. */
export class LocalTickerQueryModel extends TickerQueryModel {
  constructor(tickers: readonly Nexus.Ticker[]) {
    super();
    this.model = new LocalQueryModel(getLabel, isEqual);
    for(const ticker of tickers) {
      this.model.add(ticker);
      this.model.add(`${ticker.symbol}.${ticker.venue}`, ticker);
    }
  }

  public async submit(query: string): Promise<readonly Nexus.Ticker[]> {
    if(!query.trim()) {
      return [];
    }
    return this.model.submit(query.trim());
  }

  private model: LocalQueryModel<Nexus.Ticker>;
}

function getLabel(ticker: Nexus.Ticker): string {
  return ticker.toString();
}

function isEqual(first: Nexus.Ticker, second: Nexus.Ticker): boolean {
  return first.equals(second);
}
