import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ProfitAndLossModel } from './profit_and_loss_model';

/** Implements the ProfitAndLossModel using HTTP requests. */
export class HttpProfitAndLossModel extends ProfitAndLossModel {

  /** Constructs an HttpProfitAndLossModel.
   *  @param account - The account to generate reports for.
   *  @param serviceClients - The clients used to access HTTP services.
   */
  constructor(
      account: Beam.DirectoryEntry, serviceClients: Nexus.ServiceClients) {
    super();
    this._account = account;
    this._serviceClients = serviceClients;
  }

  public async load(): Promise<void> {
    await this._serviceClients.open();
  }

  public async startReport(start: Beam.Date, end: Beam.Date): Promise<number> {
    const response = await Beam.post(
      '/api/reporting_service/start_profit_and_loss_report',
      {
        account: this._account.toJson(),
        start: start.toJson(),
        end: end.toJson()
      });
    return response;
  }

  public async awaitReport(id: number): Promise<ProfitAndLossModel.Report> {
    let delay = 500;
    while(true) {
      const response = await Beam.post(
        '/api/reporting_service/load_profit_and_loss_report',
        {
          account: this._account.toJson(),
          id: id
        });
      if(response.status === 'ready') {
        return parseReport(response.report);
      }
      await sleep(delay);
      delay = Math.min(delay * 2, 20000);
    }
  }

  public async cancelReport(id: number): Promise<void> {
    await Beam.post('/api/reporting_service/cancel_profit_and_loss_report',
      {
        account: this._account.toJson(),
        id: id
      });
  }

  private _account: Beam.DirectoryEntry;
  private _serviceClients: Nexus.ServiceClients;
}

function parseReport(value: any): ProfitAndLossModel.Report {
  const currencies = (value.currencies as any[]).map(parseCurrencyEntry);
  const exchangeRates =
    (value.exchange_rates as any[]).map(Nexus.ExchangeRate.fromJson);
  return {
    totalProfitAndLoss: Nexus.Money.fromJson(value.total_profit_and_loss),
    totalFees: Nexus.Money.fromJson(value.total_fees),
    totalVolume: Nexus.Quantity.fromJson(value.total_volume),
    currencies: currencies,
    exchangeRates: exchangeRates
  };
}

function parseCurrencyEntry(value: any): ProfitAndLossModel.CurrencyEntry {
  const tickers = (value.tickers as any[]).map(parseTickerEntry);
  return {
    currency: Nexus.Currency.fromJson(value.currency),
    totalProfitAndLoss: Nexus.Money.fromJson(value.total_profit_and_loss),
    totalVolume: Nexus.Quantity.fromJson(value.total_volume),
    totalFees: Nexus.Money.fromJson(value.total_fees),
    tickers: tickers
  };
}

function parseTickerEntry(value: any): ProfitAndLossModel.TickerEntry {
  return {
    ticker: Nexus.Ticker.fromJson(value.ticker),
    volume: Nexus.Quantity.fromJson(value.volume),
    fees: Nexus.Money.fromJson(value.fees),
    profitAndLoss: Nexus.Money.fromJson(value.profit_and_loss)
  };
}

function sleep(ms: number): Promise<void> {
  return new Promise(resolve => setTimeout(resolve, ms));
}
