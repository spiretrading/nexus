import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { GroupProfitAndLossModel } from './group_profit_and_loss_model';
import { GroupProfitAndLossPage } from './group_profit_and_loss_page';

interface Properties {

  /** The account's base currency. */
  currency: Nexus.Currency;

  /** The database of currencies. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The model to use. */
  model: GroupProfitAndLossModel;
}

interface State {
  status: GroupProfitAndLossPage.Status;
  previousStatus: GroupProfitAndLossPage.Status;
  mode: GroupProfitAndLossPage.Mode;
  startDate: Beam.Date;
  endDate: Beam.Date;
  report: GroupProfitAndLossModel.Report;
}

/** Implements a controller for the GroupProfitAndLossPage. */
export class GroupProfitAndLossController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const today = Beam.Date.today();
    this.state = {
      status: GroupProfitAndLossPage.Status.EMPTY,
      previousStatus: GroupProfitAndLossPage.Status.EMPTY,
      mode: GroupProfitAndLossPage.Mode.THIS_MONTH,
      startDate: new Beam.Date(today.year, today.month, 1),
      endDate: today,
      report: null
    };
    this.pendingReportId = null;
    this.downloadUrl = null;
  }

  public render(): JSX.Element {
    const accountEntry =
      this.props.currencyDatabase.fromCurrency(this.props.currency);
    const report = this.state.report;
    return (
      <GroupProfitAndLossPage
        symbol={accountEntry.sign}
        code={accountEntry.code}
        status={this.state.status}
        previousStatus={this.state.previousStatus}
        mode={this.state.mode}
        startDate={this.state.startDate}
        endDate={this.state.endDate}
        totalPnl={report?.totalProfitAndLoss.toLocaleString() ?? ''}
        totalFees={report?.totalFees.toLocaleString() ?? ''}
        totalVolume={report?.totalVolume.toLocaleString() ?? ''}
        accounts={report ? this.toAccountEntries(report.accounts) : []}
        foreignCurrencies={
          report ? this.toExchangeRates(report.exchangeRates) : []}
        filepath={this.downloadUrl ?? ''}
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

  private onModeChange = (mode: GroupProfitAndLossPage.Mode) => {
    const status = this.getStaleStatus();
    if(mode === GroupProfitAndLossPage.Mode.THIS_MONTH) {
      const today = Beam.Date.today();
      this.setState({
        mode, status,
        startDate: new Beam.Date(today.year, today.month, 1),
        endDate: today
      });
    } else if(mode === GroupProfitAndLossPage.Mode.LAST_MONTH) {
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
  };

  private onStartDateChange = (startDate: Beam.Date) => {
    this.setState({startDate, status: this.getStaleStatus()});
  };

  private onEndDateChange = (endDate: Beam.Date) => {
    this.setState({endDate, status: this.getStaleStatus()});
  };

  private getStaleStatus(): GroupProfitAndLossPage.Status {
    if(this.state.report !== null &&
        this.state.status !== GroupProfitAndLossPage.Status.IN_PROGRESS) {
      return GroupProfitAndLossPage.Status.STALE;
    }
    return this.state.status;
  }

  private onSubmit = async (start: Beam.Date, end: Beam.Date) => {
    this.setState(state => ({
      previousStatus: state.status,
      status: GroupProfitAndLossPage.Status.IN_PROGRESS,
      startDate: start,
      endDate: end
    }));
    try {
      const id = await this.props.model.startReport(start, end);
      this.pendingReportId = id;
      const report = await this.props.model.awaitReport(id);
      this.pendingReportId = null;
      this.revokeDownloadUrl();
      this.downloadUrl = this.createDownloadUrl(report);
      this.setState({
        status: report.accounts.length > 0 ?
          GroupProfitAndLossPage.Status.READY :
          GroupProfitAndLossPage.Status.NO_RESULTS,
        report
      });
    } catch {
      if(this.pendingReportId !== null) {
        this.pendingReportId = null;
        this.setState({status: GroupProfitAndLossPage.Status.ERROR});
      }
    }
  };

  private onCancel = async () => {
    const id = this.pendingReportId;
    this.pendingReportId = null;
    this.setState(state => ({
      status: state.previousStatus
    }));
    if(id !== null) {
      await this.props.model.cancelReport(id);
    }
  };

  private createDownloadUrl(report: GroupProfitAndLossModel.Report): string {
    const csv = GroupProfitAndLossModel.toCsv(
      report, this.props.currency, this.props.currencyDatabase);
    const blob = new Blob([csv], {type: 'text/csv'});
    return URL.createObjectURL(blob);
  }

  private revokeDownloadUrl(): void {
    if(this.downloadUrl) {
      URL.revokeObjectURL(this.downloadUrl);
      this.downloadUrl = null;
    }
  }

  private toAccountEntries(entries: GroupProfitAndLossModel.AccountEntry[]):
      GroupProfitAndLossPage.AccountEntry[] {
    return entries.map(entry => ({
      username: entry.account.name,
      totalPnl: entry.totalProfitAndLoss.toLocaleString(),
      currencies: this.toCurrencyEntries(entry.currencies)
    }));
  }

  private toCurrencyEntries(entries: GroupProfitAndLossModel.CurrencyEntry[]):
      GroupProfitAndLossPage.CurrencyEntry[] {
    return entries.map(entry => {
      const currencyEntry =
        this.props.currencyDatabase.fromCurrency(entry.currency);
      return {
        symbol: currencyEntry.sign,
        code: currencyEntry.code,
        totalProfitAndLoss: entry.totalProfitAndLoss,
        totalVolume: entry.totalVolume,
        totalFees: entry.totalFees,
        tickers: entry.tickers
      };
    });
  }

  private toExchangeRates(rates: Nexus.ExchangeRate[]):
      GroupProfitAndLossPage.ExchangeRate[] {
    return rates.map(rate => {
      const entry = this.props.currencyDatabase.fromCurrency(rate.pair.base);
      return {
        code: entry.code,
        symbol: entry.sign,
        rate: rate.rate.valueOf().toFixed(2)
      };
    });
  }

  private pendingReportId: number;
  private downloadUrl: string;
}
