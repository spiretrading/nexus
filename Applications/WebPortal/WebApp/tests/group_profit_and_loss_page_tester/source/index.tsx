import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const TSX = Nexus.DefaultVenues.TSX;
const NASDAQ = Nexus.DefaultVenues.NASDAQ;
const ASX = Nexus.DefaultVenues.ASX;

const ACCOUNTS: WebPortal.GroupProfitAndLossPage.AccountEntry[] = [
  {
    username: 'trader_one',
    totalPnl: '4,312.75',
    currencies: [
      {
        symbol: '$',
        code: 'CAD',
        totalProfitAndLoss: Nexus.Money.parse('2769.95'),
        totalVolume: Nexus.Quantity.parse('31800'),
        totalFees: Nexus.Money.parse('318.00'),
        tickers: [
          {ticker: new Nexus.Ticker('RY', TSX),
            volume: Nexus.Quantity.parse('12450'),
            fees: Nexus.Money.parse('124.50'),
            profitAndLoss: Nexus.Money.parse('3287.15')},
          {ticker: new Nexus.Ticker('TD', TSX),
            volume: Nexus.Quantity.parse('8300'),
            fees: Nexus.Money.parse('83.00'),
            profitAndLoss: Nexus.Money.parse('-1542.80')},
          {ticker: new Nexus.Ticker('BNS', TSX),
            volume: Nexus.Quantity.parse('5200'),
            fees: Nexus.Money.parse('52.00'),
            profitAndLoss: Nexus.Money.parse('891.33')},
          {ticker: new Nexus.Ticker('ENB', TSX),
            volume: Nexus.Quantity.parse('3100'),
            fees: Nexus.Money.parse('31.00'),
            profitAndLoss: Nexus.Money.parse('445.67')},
          {ticker: new Nexus.Ticker('CNR', TSX),
            volume: Nexus.Quantity.parse('2750'),
            fees: Nexus.Money.parse('27.50'),
            profitAndLoss: Nexus.Money.parse('-312.40')}
        ]
      },
      {
        symbol: '$',
        code: 'USD',
        totalProfitAndLoss: Nexus.Money.parse('1542.80'),
        totalVolume: Nexus.Quantity.parse('8500'),
        totalFees: Nexus.Money.parse('85.00'),
        tickers: [
          {ticker: new Nexus.Ticker('AAPL', NASDAQ),
            volume: Nexus.Quantity.parse('4800'),
            fees: Nexus.Money.parse('48.00'),
            profitAndLoss: Nexus.Money.parse('1145.90')},
          {ticker: new Nexus.Ticker('MSFT', NASDAQ),
            volume: Nexus.Quantity.parse('3700'),
            fees: Nexus.Money.parse('37.00'),
            profitAndLoss: Nexus.Money.parse('396.90')}
        ]
      }
    ]
  },
  {
    username: 'trader_two',
    totalPnl: '2,150.19',
    currencies: [
      {
        symbol: '$',
        code: 'USD',
        totalProfitAndLoss: Nexus.Money.parse('1859.34'),
        totalVolume: Nexus.Quantity.parse('4000'),
        totalFees: Nexus.Money.parse('40.00'),
        tickers: [
          {ticker: new Nexus.Ticker('GOOGL', NASDAQ),
            volume: Nexus.Quantity.parse('2500'),
            fees: Nexus.Money.parse('25.00'),
            profitAndLoss: Nexus.Money.parse('1223.44')},
          {ticker: new Nexus.Ticker('TSLA', NASDAQ),
            volume: Nexus.Quantity.parse('1500'),
            fees: Nexus.Money.parse('15.00'),
            profitAndLoss: Nexus.Money.parse('635.90')}
        ]
      },
      {
        symbol: '$',
        code: 'AUD',
        totalProfitAndLoss: Nexus.Money.parse('290.85'),
        totalVolume: Nexus.Quantity.parse('2100'),
        totalFees: Nexus.Money.parse('21.00'),
        tickers: [
          {ticker: new Nexus.Ticker('BHP', ASX),
            volume: Nexus.Quantity.parse('1200'),
            fees: Nexus.Money.parse('12.00'),
            profitAndLoss: Nexus.Money.parse('190.55')},
          {ticker: new Nexus.Ticker('CBA', ASX),
            volume: Nexus.Quantity.parse('900'),
            fees: Nexus.Money.parse('9.00'),
            profitAndLoss: Nexus.Money.parse('100.30')}
        ]
      }
    ]
  },
  {
    username: 'trader_three',
    totalPnl: '1,100.00',
    currencies: [
      {
        symbol: '$',
        code: 'CAD',
        totalProfitAndLoss: Nexus.Money.parse('1100.00'),
        totalVolume: Nexus.Quantity.parse('9500'),
        totalFees: Nexus.Money.parse('95.00'),
        tickers: [
          {ticker: new Nexus.Ticker('SU', TSX),
            volume: Nexus.Quantity.parse('5000'),
            fees: Nexus.Money.parse('50.00'),
            profitAndLoss: Nexus.Money.parse('780.00')},
          {ticker: new Nexus.Ticker('CP', TSX),
            volume: Nexus.Quantity.parse('4500'),
            fees: Nexus.Money.parse('45.00'),
            profitAndLoss: Nexus.Money.parse('320.00')}
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

const Status = WebPortal.GroupProfitAndLossPage.Status;
type Status = WebPortal.GroupProfitAndLossPage.Status;

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
          } else if(status === Status.READY || status === Status.STALE) {
            this._controllerRef.current?.setState({
              status,
              totalPnl: '7,562.94',
              totalFees: '498.00',
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
