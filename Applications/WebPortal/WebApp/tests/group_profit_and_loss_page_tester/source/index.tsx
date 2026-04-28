import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const Status = WebPortal.GroupProfitAndLossPage.Status;
type Status = WebPortal.GroupProfitAndLossPage.Status;

const ACCOUNTS: WebPortal.GroupProfitAndLossPage.AccountEntry[] = [
  {
    username: 'trader_one',
    totalPnl: '$4,312.75',
    currencies: [
      {
        symbol: '$',
        code: 'CAD',
        totalProfitAndLoss: '$2,769.95',
        totalVolume: '31,800',
        totalFees: '$318.00',
        tickers: [
          {symbol: 'RY', volume: '12,450', fees: '$124.50',
            pnl: '$3,287.15'},
          {symbol: 'TD', volume: '8,300', fees: '$83.00',
            pnl: '-$1,542.80'},
          {symbol: 'BNS', volume: '5,200', fees: '$52.00',
            pnl: '$891.33'},
          {symbol: 'ENB', volume: '3,100', fees: '$31.00',
            pnl: '$445.67'},
          {symbol: 'CNR', volume: '2,750', fees: '$27.50',
            pnl: '-$312.40'}
        ]
      },
      {
        symbol: '$',
        code: 'USD',
        totalProfitAndLoss: '$1,542.80',
        totalVolume: '8,500',
        totalFees: '$85.00',
        tickers: [
          {symbol: 'AAPL', volume: '4,800', fees: '$48.00',
            pnl: '$1,145.90'},
          {symbol: 'MSFT', volume: '3,700', fees: '$37.00',
            pnl: '$396.90'}
        ]
      }
    ]
  },
  {
    username: 'trader_two',
    totalPnl: '$2,150.19',
    currencies: [
      {
        symbol: '$',
        code: 'USD',
        totalProfitAndLoss: '$1,859.34',
        totalVolume: '4,000',
        totalFees: '$40.00',
        tickers: [
          {symbol: 'GOOGL', volume: '2,500', fees: '$25.00',
            pnl: '$1,223.44'},
          {symbol: 'TSLA', volume: '1,500', fees: '$15.00',
            pnl: '$635.90'}
        ]
      },
      {
        symbol: '$',
        code: 'AUD',
        totalProfitAndLoss: '$290.85',
        totalVolume: '2,100',
        totalFees: '$21.00',
        tickers: [
          {symbol: 'BHP', volume: '1,200', fees: '$12.00',
            pnl: '$190.55'},
          {symbol: 'CBA', volume: '900', fees: '$9.00',
            pnl: '$100.30'}
        ]
      }
    ]
  },
  {
    username: 'trader_three',
    totalPnl: '$1,100.00',
    currencies: [
      {
        symbol: '$',
        code: 'CAD',
        totalProfitAndLoss: '$1,100.00',
        totalVolume: '9,500',
        totalFees: '$95.00',
        tickers: [
          {symbol: 'SU', volume: '5,000', fees: '$50.00',
            pnl: '$780.00'},
          {symbol: 'CP', volume: '4,500', fees: '$45.00',
            pnl: '$320.00'}
        ]
      }
    ]
  }
];

const FOREIGN_CURRENCIES:
    WebPortal.GroupProfitAndLossPage.ExchangeRate[] = [
  {code: 'USD', symbol: '$', rate: '1.36'},
  {code: 'AUD', symbol: '$', rate: '0.88'}
];

interface State {
  forceStatus: Status;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      forceStatus: null
    };
    this._controllerRef = React.createRef();
  }

  public render(): JSX.Element {
    return (
      <div style={STYLE.wrapper}>
        <div style={STYLE.toolbar}>
          <span style={STYLE.toolbarLabel}>Force:</span>
          {this.renderForceButton('NONE', null)}
          {this.renderForceButton('EMPTY', Status.EMPTY)}
          {this.renderForceButton('READY', Status.READY)}
          {this.renderForceButton('STALE', Status.STALE)}
          {this.renderForceButton('IN_PROGRESS', Status.IN_PROGRESS)}
          {this.renderForceButton('NO_RESULTS', Status.NO_RESULTS)}
          {this.renderForceButton('ERROR', Status.ERROR)}
        </div>
        <WebPortal.GroupProfitAndLossController
          ref={this._controllerRef}
          symbol='$'
          code='CAD'/>
      </div>);
  }

  private renderForceButton(label: string, status: Status): JSX.Element {
    const isActive = this.state.forceStatus === status;
    return (
      <button key={label}
        style={{...STYLE.button, ...(isActive && STYLE.buttonActive)}}
        onClick={() => {
          this.setState({forceStatus: status});
          if(status === null) {
            this._controllerRef.current?.setState({
              status: Status.EMPTY,
              accounts: []
            } as any);
          } else if(status === Status.READY) {
            this._controllerRef.current?.setState({
              status,
              totalPnl: '$7,562.94',
              totalFees: '$498.00',
              totalVolume: '49,800',
              accounts: ACCOUNTS,
              foreignCurrencies: FOREIGN_CURRENCIES,
              filepath: '/reports/group-pl.csv'
            } as any);
          } else if(status === Status.STALE) {
            this._controllerRef.current?.setState({
              status,
              totalPnl: '$7,562.94',
              totalFees: '$498.00',
              totalVolume: '49,800',
              accounts: ACCOUNTS,
              foreignCurrencies: FOREIGN_CURRENCIES,
              filepath: '/reports/group-pl.csv'
            } as any);
          } else {
            this._controllerRef.current?.setState({
              status,
              accounts: []
            } as any);
          }
        }}>
        {label}
      </button>);
  }

  private _controllerRef:
    React.RefObject<WebPortal.GroupProfitAndLossController>;
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
  button: {
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
  buttonActive: {
    backgroundColor: '#684BC7',
    color: '#fff',
    borderColor: '#684BC7'
  }
};

ReactDOM.render(<TestApp/>, document.getElementById('main'));
