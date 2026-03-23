import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Model used by the ProfitAndLossPage. */
export abstract class ProfitAndLossModel {

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
  public abstract awaitReport(id: number): Promise<ProfitAndLossModel.Report>;

  /** Cancels an in-progress report.
   *  @param id - The report identifier to cancel.
   */
  public abstract cancelReport(id: number): Promise<void>;
}

export namespace ProfitAndLossModel {

  /** A per-security breakdown within a currency. */
  export interface SecurityEntry {

    /** The security traded. */
    security: Nexus.Security;

    /** The volume traded. */
    volume: number;

    /** The fees incurred. */
    fees: Nexus.Money;

    /** The profit and loss. */
    profitAndLoss: Nexus.Money;
  }

  /** A per-currency breakdown within a report. */
  export interface CurrencyEntry {

    /** The currency. */
    currency: Nexus.Currency;

    /** The total profit and loss for this currency. */
    totalProfitAndLoss: Nexus.Money;

    /** The total volume traded for this currency. */
    totalVolume: number;

    /** The total fees incurred for this currency. */
    totalFees: Nexus.Money;

    /** The securities traded in this currency. */
    securities: SecurityEntry[];
  }

  /** The result of a completed report. */
  export interface Report {

    /** The total profit and loss in account currency. */
    totalProfitAndLoss: Nexus.Money;

    /** The total fees in account currency. */
    totalFees: Nexus.Money;

    /** The total volume in account currency. */
    totalVolume: number;

    /** The list of currencies with activity. */
    currencies: CurrencyEntry[];

    /** The exchange rates for foreign currencies. */
    exchangeRates: Nexus.ExchangeRate[];
  }

  /** Generates a CSV string from a report.
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
    rows.push(['Currency', 'Security', 'Volume', 'Fees', 'Profit/Loss']);
    for(const currency of report.currencies) {
      const currencyEntry = currencyDatabase.fromCurrency(currency.currency);
      for(const security of currency.securities) {
        rows.push([
          currencyEntry.code,
          security.security.toString(),
          security.volume.toString(),
          currencyEntry.sign + security.fees.toString(),
          currencyEntry.sign + security.profitAndLoss.toString()
        ]);
      }
      rows.push([
        currencyEntry.code,
        'Total',
        currency.totalVolume.toString(),
        currencyEntry.sign + currency.totalFees.toString(),
        currencyEntry.sign + currency.totalProfitAndLoss.toString()
      ]);
    }
    rows.push([
      'Total (' + accountEntry.code + ')',
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
