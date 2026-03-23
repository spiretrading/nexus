import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const Status = WebPortal.ProfitAndLossPage.Status;
type Status = WebPortal.ProfitAndLossPage.Status;
const Mode = WebPortal.ProfitAndLossPage.Mode;

const SECURITIES_CAD: WebPortal.ProfitAndLossTable.Security[] = [
  {symbol: 'RY', volume: '12,450', fees: '124.50', pnl: '3,287.15'},
  {symbol: 'TD', volume: '8,300', fees: '83.00', pnl: '-1,542.80'},
  {symbol: 'BNS', volume: '5,200', fees: '52.00', pnl: '891.33'},
  {symbol: 'ENB', volume: '3,100', fees: '31.00', pnl: '445.67'},
  {symbol: 'CNR', volume: '2,750', fees: '27.50', pnl: '-312.40'}
];

const SECURITIES_USD: WebPortal.ProfitAndLossTable.Security[] = [
  {symbol: 'AAPL', volume: '6,800', fees: '68.00', pnl: '2,145.90'},
  {symbol: 'MSFT', volume: '4,500', fees: '45.00', pnl: '1,823.44'},
  {symbol: 'GOOGL', volume: '1,200', fees: '12.00', pnl: '-567.20'}
];

const SECURITIES_AUD: WebPortal.ProfitAndLossTable.Security[] = [
  {symbol: 'BHP', volume: '3,400', fees: '34.00', pnl: '912.55'},
  {symbol: 'CBA', volume: '2,100', fees: '21.00', pnl: '478.30'}
];

const CURRENCIES: WebPortal.ProfitAndLossPage.CurrencyEntry[] = [
  {
    symbol: '$',
    code: 'CAD',
    totalPnl: '2,769.95',
    totalVolume: '31,800',
    totalFees: '318.00',
    securities: SECURITIES_CAD
  },
  {
    symbol: '$',
    code: 'USD',
    totalPnl: '3,402.14',
    totalVolume: '12,500',
    totalFees: '125.00',
    securities: SECURITIES_USD
  },
  {
    symbol: '$',
    code: 'AUD',
    totalPnl: '1,390.85',
    totalVolume: '5,500',
    totalFees: '55.00',
    securities: SECURITIES_AUD
  }
];

const EXCHANGE_RATES: WebPortal.CurrencyTooltip.ExchangeRate[] = [
  {code: 'USD', rate: '1.36'},
  {code: 'AUD', rate: '0.88'}
];

interface State {
  status: Status;
  previousStatus: Status;
  mode: WebPortal.ProfitAndLossPage.Mode;
  startDate: Beam.Date;
  endDate: Beam.Date;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    const today = Beam.Date.today();
    this.state = {
      status: Status.NONE,
      previousStatus: Status.NONE,
      mode: Mode.PRESET,
      startDate: new Beam.Date(today.year, today.month, 1),
      endDate: today
    };
  }

  public render(): JSX.Element {
    const hasData = this.state.status === Status.READY ||
      (this.state.status === Status.STALE && true);
    return (
      <div style={STYLE.wrapper}>
        <div style={STYLE.toolbar}>
          <span style={STYLE.toolbarLabel}>Status:</span>
          {this.renderStatusButton('NONE', Status.NONE)}
          {this.renderStatusButton('IN_PROGRESS', Status.IN_PROGRESS)}
          {this.renderStatusButton('READY', Status.READY)}
          {this.renderStatusButton('STALE', Status.STALE)}
          {this.renderStatusButton('ERROR', Status.ERROR)}
        </div>
        <WebPortal.ProfitAndLossPage
          symbol='$'
          code='CAD'
          status={this.state.status}
          previousStatus={this.state.previousStatus}
          mode={this.state.mode}
          startDate={this.state.startDate}
          endDate={this.state.endDate}
          totalPnl='7,562.94'
          totalFees='498.00'
          totalVolume='49,800'
          currencies={hasData ? CURRENCIES : []}
          foreignCurrencies={hasData ? EXCHANGE_RATES : []}
          filepath='/sample-report.csv'
          onModeChange={this.onModeChange}
          onStartDateChange={this.onStartDateChange}
          onEndDateChange={this.onEndDateChange}
          onSubmit={this.onSubmit}
          onCancel={this.onCancel}/>
      </div>);
  }

  private renderStatusButton(label: string, status: Status): JSX.Element {
    const isActive = this.state.status === status;
    return (
      <button
        key={label}
        style={{...STYLE.statusButton,
          ...(isActive && STYLE.statusButtonActive)}}
        onClick={() => this.setState(state => ({
          previousStatus: state.status,
          status
        }))}>
        {label}
      </button>);
  }

  private onModeChange = (mode: WebPortal.ProfitAndLossPage.Mode) => {
    this.setState({mode});
  }

  private onStartDateChange = (startDate: Beam.Date) => {
    this.setState({startDate});
  }

  private onEndDateChange = (endDate: Beam.Date) => {
    this.setState({endDate});
  }

  private onSubmit = (start: Beam.Date, end: Beam.Date) => {
    console.log(`Submit: ${start.toJson()} - ${end.toJson()}`);
    this.setState(state => ({
      previousStatus: state.status,
      status: Status.IN_PROGRESS,
      startDate: start,
      endDate: end
    }));
  }

  private onCancel = () => {
    console.log('Cancel');
    this.setState(state => ({
      status: state.previousStatus
    }));
  }
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column'
  },
  toolbar: {
    display: 'flex',
    alignItems: 'center',
    gap: '6px',
    padding: '8px 12px',
    backgroundColor: '#F0F0F0',
    borderBottom: '1px solid #DDD'
  },
  toolbarLabel: {
    fontSize: '12px',
    fontWeight: 600,
    fontFamily: 'monospace'
  },
  statusButton: {
    fontSize: '11px',
    fontFamily: 'monospace',
    padding: '2px 6px',
    borderWidth: '1px',
    borderStyle: 'solid',
    borderColor: '#ccc',
    borderRadius: '3px',
    backgroundColor: '#fff',
    cursor: 'pointer'
  },
  statusButtonActive: {
    backgroundColor: '#684BC7',
    color: '#fff',
    borderColor: '#684BC7'
  }
};

ReactDOM.render(<TestApp/>, document.getElementById('main'));
