import * as Beam from 'beam';
import * as React from 'react';
import { GroupProfitAndLossPage } from './group_profit_and_loss_page';

interface Properties {

  /** The account's currency symbol. */
  symbol: string;

  /** The account's currency code. */
  code: string;
}

interface State {
  status: GroupProfitAndLossPage.Status;
  previousStatus: GroupProfitAndLossPage.Status;
  mode: GroupProfitAndLossPage.Mode;
  startDate: Beam.Date;
  endDate: Beam.Date;
  totalPnl: string;
  totalFees: string;
  totalVolume: string;
  accounts: GroupProfitAndLossPage.AccountEntry[];
  foreignCurrencies: GroupProfitAndLossPage.ExchangeRate[];
  filepath: string;
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
      totalPnl: '',
      totalFees: '',
      totalVolume: '',
      accounts: [],
      foreignCurrencies: [],
      filepath: ''
    };
  }

  public render(): JSX.Element {
    return (
      <GroupProfitAndLossPage
        symbol={this.props.symbol}
        code={this.props.code}
        status={this.state.status}
        previousStatus={this.state.previousStatus}
        mode={this.state.mode}
        startDate={this.state.startDate}
        endDate={this.state.endDate}
        totalPnl={this.state.totalPnl}
        totalFees={this.state.totalFees}
        totalVolume={this.state.totalVolume}
        accounts={this.state.accounts}
        foreignCurrencies={this.state.foreignCurrencies}
        filepath={this.state.filepath}
        onModeChange={this.onModeChange}
        onStartDateChange={this.onStartDateChange}
        onEndDateChange={this.onEndDateChange}
        onSubmit={this.onSubmit}
        onCancel={this.onCancel}/>);
  }

  private onModeChange = (mode: GroupProfitAndLossPage.Mode) => {
    const today = Beam.Date.today();
    if(mode === GroupProfitAndLossPage.Mode.THIS_MONTH) {
      this.setState({
        mode,
        startDate: new Beam.Date(today.year, today.month, 1),
        endDate: today
      });
    } else if(mode === GroupProfitAndLossPage.Mode.LAST_MONTH) {
      const lastMonth = today.month === 1 ?
        new Beam.Date(today.year - 1, 12, 1) :
        new Beam.Date(today.year, today.month - 1, 1);
      const lastDay = new Beam.Date(today.year, today.month, 1);
      this.setState({
        mode,
        startDate: lastMonth,
        endDate: new Beam.Date(lastDay.year, lastDay.month - 1,
          new Date(lastDay.year, lastDay.month - 1, 0).getDate())
      });
    } else {
      this.setState({mode});
    }
  };

  private onStartDateChange = (date: Beam.Date) => {
    this.setState({startDate: date});
  };

  private onEndDateChange = (date: Beam.Date) => {
    this.setState({endDate: date});
  };

  private onSubmit = (_start: Beam.Date, _end: Beam.Date) => {
  };

  private onCancel = () => {
  };
}
