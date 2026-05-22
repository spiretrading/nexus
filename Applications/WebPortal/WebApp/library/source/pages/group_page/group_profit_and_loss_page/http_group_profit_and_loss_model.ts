import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { GroupProfitAndLossModel } from './group_profit_and_loss_model';

/** Implements the GroupProfitAndLossModel using HTTP requests. */
export class HttpGroupProfitAndLossModel extends GroupProfitAndLossModel {

  /** Constructs an HttpGroupProfitAndLossModel.
   *  @param group - The group to generate reports for.
   *  @param serviceClients - The clients used to access HTTP services.
   */
  constructor(
      group: Beam.DirectoryEntry, serviceClients: Nexus.ServiceClients) {
    super();
    this.group = group;
    this.serviceClients = serviceClients;
  }

  public async load(): Promise<void> {
    await this.serviceClients.open();
  }

  public async startReport(start: Beam.Date, end: Beam.Date): Promise<number> {
    const response = await Beam.post(
      '/api/reporting_service/start_group_profit_and_loss_report',
      {
        group: this.group.toJson(),
        start: start.toJson(),
        end: end.toJson()
      });
    return response;
  }

  public async awaitReport(
      id: number): Promise<GroupProfitAndLossModel.Report> {
    let delay = 500;
    while(true) {
      const response = await Beam.post(
        '/api/reporting_service/load_group_profit_and_loss_report',
        {
          group: this.group.toJson(),
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
    await Beam.post(
      '/api/reporting_service/cancel_group_profit_and_loss_report',
      {
        group: this.group.toJson(),
        id: id
      });
  }

  private group: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
}

function parseReport(value: any): GroupProfitAndLossModel.Report {
  const accounts = (value.accounts as any[]).map(parseAccountEntry);
  const exchangeRates =
    (value.exchange_rates as any[]).map(Nexus.ExchangeRate.fromJson);
  return {
    totalProfitAndLoss: Nexus.Money.fromJson(value.total_profit_and_loss),
    totalFees: Nexus.Money.fromJson(value.total_fees),
    totalVolume: Nexus.Quantity.fromJson(value.total_volume),
    accounts: accounts,
    exchangeRates: exchangeRates
  };
}

function parseAccountEntry(
    value: any): GroupProfitAndLossModel.AccountEntry {
  const currencies =
    (value.currencies as any[]).map(parseCurrencyEntry);
  return {
    account: Beam.DirectoryEntry.fromJson(value.account),
    totalProfitAndLoss: Nexus.Money.fromJson(value.total_profit_and_loss),
    currencies: currencies
  };
}

function parseCurrencyEntry(
    value: any): GroupProfitAndLossModel.CurrencyEntry {
  const tickers = (value.tickers as any[]).map(parseTickerEntry);
  return {
    currency: Nexus.Currency.fromJson(value.currency),
    totalProfitAndLoss: Nexus.Money.fromJson(value.total_profit_and_loss),
    totalVolume: Nexus.Quantity.fromJson(value.total_volume),
    totalFees: Nexus.Money.fromJson(value.total_fees),
    tickers: tickers
  };
}

function parseTickerEntry(value: any): GroupProfitAndLossModel.TickerEntry {
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
