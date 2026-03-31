import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { CurrencyTooltip } from './currency_tooltip';
import { ProfitAndLossModel } from './profit_and_loss_model';
import { ProfitAndLossPage } from './profit_and_loss_page';
import { ProfitAndLossTable } from './profit_and_loss_table';

interface Properties {

  /** The account's base currency. */
  currency: Nexus.Currency;

  /** The database of currencies. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The model to use. */
  model: ProfitAndLossModel;
}

interface State {
  status: ProfitAndLossPage.Status;
  previousStatus: ProfitAndLossPage.Status;
  mode: ProfitAndLossPage.Mode;
  startDate: Beam.Date;
  endDate: Beam.Date;
  report: ProfitAndLossModel.Report;
}

/** Implements a controller for the ProfitAndLossPage. */
export class ProfitAndLossController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const today = Beam.Date.today();
    this.state = {
      status: ProfitAndLossPage.Status.NONE,
      previousStatus: ProfitAndLossPage.Status.NONE,
      mode: ProfitAndLossPage.Mode.THIS_MONTH,
      startDate: new Beam.Date(today.year, today.month, 1),
      endDate: today,
      report: null
    };
    this._pendingReportId = null;
    this._downloadUrl = null;
  }

  public render(): JSX.Element {
    const accountEntry = this.props.currencyDatabase.fromCurrency(
      this.props.currency);
    const report = this.state.report;
    return (
      <ProfitAndLossPage
        symbol={accountEntry.sign}
        code={accountEntry.code}
        status={this.state.status}
        previousStatus={this.state.previousStatus}
        mode={this.state.mode}
        startDate={this.state.startDate}
        endDate={this.state.endDate}
        totalPnl={report?.totalProfitAndLoss.toString() ?? ''}
        totalFees={report?.totalFees.toString() ?? ''}
        totalVolume={report?.totalVolume.toString() ?? ''}
        currencies={report ? this.toCurrencyEntries(report.currencies) : []}
        foreignCurrencies={report ?
          this.toExchangeRates(report.exchangeRates) : []}
        filepath={this._downloadUrl ?? ''}
        onModeChange={this.onModeChange}
        onStartDateChange={this.onStartDateChange}
        onEndDateChange={this.onEndDateChange}
        onSubmit={this.onSubmit}
        onCancel={this.onCancel}/>);
  }

  public async componentDidMount(): Promise<void> {
    await this.props.model.load();
  }

  public componentWillUnmount(): void {
    this.revokeDownloadUrl();
  }

  private onModeChange = (mode: ProfitAndLossPage.Mode) => {
    const status = this.getStaleStatus();
    if(mode === ProfitAndLossPage.Mode.THIS_MONTH) {
      const today = Beam.Date.today();
      this.setState({
        mode, status,
        startDate: new Beam.Date(today.year, today.month, 1),
        endDate: today
      });
    } else if(mode === ProfitAndLossPage.Mode.LAST_MONTH) {
      const today = Beam.Date.today();
      const year = today.month === 1 ? today.year - 1 : today.year;
      const month = today.month === 1 ? 12 : today.month - 1;
      const lastDay = new globalThis.Date(year, month, 0).getDate();
      this.setState({
        mode, status,
        startDate: new Beam.Date(year, month, 1),
        endDate: new Beam.Date(year, month, lastDay)
      });
    } else {
      this.setState({mode, status});
    }
  }

  private onStartDateChange = (startDate: Beam.Date) => {
    this.setState({startDate, status: this.getStaleStatus()});
  }

  private onEndDateChange = (endDate: Beam.Date) => {
    this.setState({endDate, status: this.getStaleStatus()});
  }

  private getStaleStatus(): ProfitAndLossPage.Status {
    if(this.state.report !== null &&
        this.state.status !== ProfitAndLossPage.Status.IN_PROGRESS) {
      return ProfitAndLossPage.Status.STALE;
    }
    return this.state.status;
  }

  private onSubmit = async (start: Beam.Date, end: Beam.Date) => {
    this.setState(state => ({
      previousStatus: state.status,
      status: ProfitAndLossPage.Status.IN_PROGRESS,
      startDate: start,
      endDate: end
    }));
    try {
      const id = await this.props.model.startReport(start, end);
      this._pendingReportId = id;
      const report = await this.props.model.awaitReport(id);
      this._pendingReportId = null;
      this.revokeDownloadUrl();
      this._downloadUrl = this.createDownloadUrl(report);
      this.setState({
        status: ProfitAndLossPage.Status.READY,
        report
      });
    } catch {
      if(this._pendingReportId !== null) {
        this._pendingReportId = null;
        this.setState({status: ProfitAndLossPage.Status.ERROR});
      }
    }
  }

  private onCancel = async () => {
    const id = this._pendingReportId;
    this._pendingReportId = null;
    this.setState(state => ({
      status: state.previousStatus
    }));
    if(id !== null) {
      await this.props.model.cancelReport(id);
    }
  }

  private createDownloadUrl(report: ProfitAndLossModel.Report): string {
    const csv = ProfitAndLossModel.toCsv(report, this.props.currency,
      this.props.currencyDatabase);
    const blob = new Blob([csv], {type: 'text/csv'});
    return URL.createObjectURL(blob);
  }

  private revokeDownloadUrl(): void {
    if(this._downloadUrl) {
      URL.revokeObjectURL(this._downloadUrl);
      this._downloadUrl = null;
    }
  }

  private toCurrencyEntries(
      entries: ProfitAndLossModel.CurrencyEntry[]):
      ProfitAndLossPage.CurrencyEntry[] {
    return entries.map(entry => {
      const currencyEntry = this.props.currencyDatabase.fromCurrency(
        entry.currency);
      return {
        symbol: currencyEntry.sign,
        code: currencyEntry.code,
        totalPnl: entry.totalProfitAndLoss.toString(),
        totalVolume: entry.totalVolume.toString(),
        totalFees: entry.totalFees.toString(),
        securities: this.toSecurities(entry.securities)
      };
    });
  }

  private toSecurities(entries: ProfitAndLossModel.SecurityEntry[]):
      ProfitAndLossTable.Security[] {
    return entries.map(entry => ({
      symbol: entry.security.toString(),
      volume: entry.volume.toString(),
      fees: entry.fees.toString(),
      pnl: entry.profitAndLoss.toString()
    }));
  }

  private toExchangeRates(
      rates: Nexus.ExchangeRate[]): CurrencyTooltip.ExchangeRate[] {
    return rates.map(rate => {
      const entry = this.props.currencyDatabase.fromCurrency(
        rate.pair.base);
      return {
        code: entry.code,
        rate: rate.rate.valueOf().toFixed(2)
      };
    });
  }

  private _pendingReportId: number;
  private _downloadUrl: string;
}
