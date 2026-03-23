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

    /** The filepath to the generated report file. */
    filepath: string;
  }
}
