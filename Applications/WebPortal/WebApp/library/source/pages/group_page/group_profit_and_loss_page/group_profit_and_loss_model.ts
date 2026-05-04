import * as Beam from 'beam';
import * as Nexus from 'nexus';


/** Model used by the GroupProfitAndLossPage. */
export abstract class GroupProfitAndLossModel {

  /** Loads this model. */
  public abstract load(): Promise<void>;

  /** Starts generating a report for the given date range.
   *  @param start - The start date of the report.
   *  @param end - The end date of the report.
   *  @return An identifier for the pending report.
   */
  public abstract startReport(
    start: Beam.Date, end: Beam.Date): Promise<number>;

  /** Waits for a report to complete and returns the result.
   *  @param id - The report identifier returned by startReport.
   *  @return The completed report data.
   */
  public abstract awaitReport(
    id: number): Promise<GroupProfitAndLossModel.Report>;

  /** Cancels an in-progress report.
   *  @param id - The report identifier to cancel.
   */
  public abstract cancelReport(id: number): Promise<void>;
}

export namespace GroupProfitAndLossModel {

  /** A per-ticker breakdown within a currency. */
  export interface TickerEntry {

    /** The ticker traded. */
    ticker: Nexus.Ticker;

    /** The volume traded. */
    volume: Nexus.Quantity;

    /** The fees incurred. */
    fees: Nexus.Money;

    /** The profit and loss. */
    profitAndLoss: Nexus.Money;
  }

  /** A per-currency breakdown within an account. */
  export interface CurrencyEntry {

    /** The currency. */
    currency: Nexus.Currency;

    /** The total profit and loss for this currency. */
    totalProfitAndLoss: Nexus.Money;

    /** The total volume traded for this currency. */
    totalVolume: Nexus.Quantity;

    /** The total fees incurred for this currency. */
    totalFees: Nexus.Money;

    /** The tickers traded in this currency. */
    tickers: TickerEntry[];
  }

  /** A per-account breakdown within a group report. */
  export interface AccountEntry {

    /** The account. */
    account: Beam.DirectoryEntry;

    /** The total profit and loss in account currency. */
    totalProfitAndLoss: Nexus.Money;

    /** The list of currencies with activity. */
    currencies: CurrencyEntry[];
  }

  /** The result of a completed group report. */
  export interface Report {

    /** The total profit and loss in account currency. */
    totalProfitAndLoss: Nexus.Money;

    /** The total fees in account currency. */
    totalFees: Nexus.Money;

    /** The total volume in account currency. */
    totalVolume: Nexus.Quantity;

    /** The list of accounts with activity. */
    accounts: AccountEntry[];

    /** The exchange rates for foreign currencies. */
    exchangeRates: Nexus.ExchangeRate[];
  }

  /** Generates a CSV string from a group report.
   *  @param report - The report data.
   *  @param accountCurrency - The account's base currency.
   *  @param currencyDatabase - The database used to look up currency codes
   *                            and signs.
   *  @return The CSV content as a string.
   */
  export function toCsv(report: Report, accountCurrency: Nexus.Currency,
      currencyDatabase: Nexus.CurrencyDatabase): string {
    const accountEntry = currencyDatabase.fromCurrency(accountCurrency);
    const rows: string[][] = [];
    rows.push(
      ['Username', 'Currency', 'Ticker', 'Volume', 'Fees', 'Profit/Loss']);
    for(const account of report.accounts) {
      for(const currency of account.currencies) {
        const currencyEntry =
          currencyDatabase.fromCurrency(currency.currency);
        for(const ticker of currency.tickers) {
          rows.push([
            account.account.name,
            currencyEntry.code,
            ticker.ticker.toString(),
            ticker.volume.toString(),
            currencyEntry.sign + ticker.fees.toString(),
            currencyEntry.sign + ticker.profitAndLoss.toString()
          ]);
        }
        rows.push([
          account.account.name,
          currencyEntry.code,
          'Total',
          currency.totalVolume.toString(),
          currencyEntry.sign + currency.totalFees.toString(),
          currencyEntry.sign + currency.totalProfitAndLoss.toString()
        ]);
      }
    }
    rows.push([
      'Total (' + accountEntry.code + ')',
      '',
      '',
      report.totalVolume.toString(),
      accountEntry.sign + report.totalFees.toString(),
      accountEntry.sign + report.totalProfitAndLoss.toString()
    ]);
    return rows.map(row => row.map(escapeCsvField).join(',')).join('\n');
  }
}

function escapeCsvField(value: string): string {
  if(value.indexOf(',') !== -1 || value.indexOf('"') !== -1 ||
      value.indexOf('\n') !== -1) {
    return '"' + value.replace(/"/g, '""') + '"';
  }
  return value;
}
