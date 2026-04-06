import * as Beam from 'beam';
import Fraction from 'fraction.js';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const CAD = new Nexus.Currency(124);
const USD = new Nexus.Currency(840);
const AUD = new Nexus.Currency(36);

const currencyDatabase = new Nexus.CurrencyDatabase();
currencyDatabase.add(new Nexus.CurrencyDatabase.Entry(CAD, 'CAD', '$'));
currencyDatabase.add(new Nexus.CurrencyDatabase.Entry(USD, 'USD', '$'));
currencyDatabase.add(new Nexus.CurrencyDatabase.Entry(AUD, 'AUD', '$'));

const TSX = Nexus.DefaultMarkets.TSX;
const NASDAQ = Nexus.DefaultMarkets.NASDAQ;
const ASX = Nexus.DefaultMarkets.ASX;

const REPORT: WebPortal.ProfitAndLossModel.Report = {
  totalProfitAndLoss: Nexus.Money.parse('7562.94'),
  totalFees: Nexus.Money.parse('498.00'),
  totalVolume: Nexus.Quantity.parse('49800'),
  currencies: [
    {
      currency: CAD,
      totalProfitAndLoss: Nexus.Money.parse('2769.95'),
      totalVolume: Nexus.Quantity.parse('31800'),
      totalFees: Nexus.Money.parse('318.00'),
      securities: [
        {security: new Nexus.Security('RY', TSX), volume: Nexus.Quantity.parse('12450'),
          fees: Nexus.Money.parse('124.50'),
          profitAndLoss: Nexus.Money.parse('3287.15')},
        {security: new Nexus.Security('TD', TSX), volume: Nexus.Quantity.parse('8300'),
          fees: Nexus.Money.parse('83.00'),
          profitAndLoss: Nexus.Money.parse('-1542.80')},
        {security: new Nexus.Security('BNS', TSX), volume: Nexus.Quantity.parse('5200'),
          fees: Nexus.Money.parse('52.00'),
          profitAndLoss: Nexus.Money.parse('891.33')},
        {security: new Nexus.Security('ENB', TSX), volume: Nexus.Quantity.parse('3100'),
          fees: Nexus.Money.parse('31.00'),
          profitAndLoss: Nexus.Money.parse('445.67')},
        {security: new Nexus.Security('CNR', TSX), volume: Nexus.Quantity.parse('2750'),
          fees: Nexus.Money.parse('27.50'),
          profitAndLoss: Nexus.Money.parse('-312.40')}
      ]
    },
    {
      currency: USD,
      totalProfitAndLoss: Nexus.Money.parse('3402.14'),
      totalVolume: Nexus.Quantity.parse('12500'),
      totalFees: Nexus.Money.parse('125.00'),
      securities: [
        {security: new Nexus.Security('AAPL', NASDAQ), volume: Nexus.Quantity.parse('6800'),
          fees: Nexus.Money.parse('68.00'),
          profitAndLoss: Nexus.Money.parse('2145.90')},
        {security: new Nexus.Security('MSFT', NASDAQ), volume: Nexus.Quantity.parse('4500'),
          fees: Nexus.Money.parse('45.00'),
          profitAndLoss: Nexus.Money.parse('1823.44')},
        {security: new Nexus.Security('GOOGL', NASDAQ), volume: Nexus.Quantity.parse('1200'),
          fees: Nexus.Money.parse('12.00'),
          profitAndLoss: Nexus.Money.parse('-567.20')}
      ]
    },
    {
      currency: AUD,
      totalProfitAndLoss: Nexus.Money.parse('1390.85'),
      totalVolume: Nexus.Quantity.parse('5500'),
      totalFees: Nexus.Money.parse('55.00'),
      securities: [
        {security: new Nexus.Security('BHP', ASX), volume: Nexus.Quantity.parse('3400'),
          fees: Nexus.Money.parse('34.00'),
          profitAndLoss: Nexus.Money.parse('912.55')},
        {security: new Nexus.Security('CBA', ASX), volume: Nexus.Quantity.parse('2100'),
          fees: Nexus.Money.parse('21.00'),
          profitAndLoss: Nexus.Money.parse('478.30')}
      ]
    }
  ],
  exchangeRates: [
    new Nexus.ExchangeRate(
      new Nexus.CurrencyPair(USD, CAD), new Fraction(136, 100)),
    new Nexus.ExchangeRate(
      new Nexus.CurrencyPair(AUD, CAD), new Fraction(88, 100))
  ]
};

enum TestBehavior {
  SUCCEED,
  FAIL,
  HANG
}

class TestModel extends WebPortal.ProfitAndLossModel {
  public behavior = TestBehavior.SUCCEED;

  constructor(report: WebPortal.ProfitAndLossModel.Report, delay: number) {
    super();
    this._report = report;
    this._delay = delay;
    this._nextId = 1;
    this._pending = new Map();
  }

  public async load(): Promise<void> {
    return;
  }

  public async startReport(
      start: Beam.Date, end: Beam.Date): Promise<number> {
    const id = this._nextId++;
    this._pending.set(id, {reject: null});
    return id;
  }

  public async awaitReport(
      id: number): Promise<WebPortal.ProfitAndLossModel.Report> {
    const entry = this._pending.get(id);
    if(!entry) {
      throw new Error(`Unknown report id: ${id}`);
    }
    if(this.behavior === TestBehavior.HANG) {
      await new Promise<void>((_, reject) => {
        entry.reject = () => {
          reject(new Error('Report cancelled.'));
        };
      });
    }
    await new Promise<void>((resolve, reject) => {
      const timer = setTimeout(() => {
        if(this.behavior === TestBehavior.FAIL) {
          reject(new Error('Report failed.'));
        } else {
          resolve();
        }
      }, this._delay);
      entry.reject = () => {
        clearTimeout(timer);
        reject(new Error('Report cancelled.'));
      };
    });
    this._pending.delete(id);
    return this._report;
  }

  public async cancelReport(id: number): Promise<void> {
    const entry = this._pending.get(id);
    if(entry) {
      entry.reject?.();
      this._pending.delete(id);
    }
  }

  private _report: WebPortal.ProfitAndLossModel.Report;
  private _delay: number;
  private _nextId: number;
  private _pending: Map<number, {reject: () => void}>;
}

const model = new TestModel(REPORT, 2000);

const Status = WebPortal.ProfitAndLossPage.Status;
type Status = WebPortal.ProfitAndLossPage.Status;

interface State {
  behavior: TestBehavior;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      behavior: TestBehavior.SUCCEED
    };
    this._controllerRef = React.createRef();
  }

  public render(): JSX.Element {
    return (
      <div style={STYLE.wrapper}>
        <div style={STYLE.toolbar}>
          <span style={STYLE.toolbarLabel}>Behavior:</span>
          {this.renderBehaviorButton('Succeed', TestBehavior.SUCCEED)}
          {this.renderBehaviorButton('Fail', TestBehavior.FAIL)}
          {this.renderBehaviorButton('Hang', TestBehavior.HANG)}
          <span style={STYLE.toolbarSeparator}>|</span>
          <span style={STYLE.toolbarLabel}>Force:</span>
          {this.renderForceButton('NONE', Status.NONE)}
          {this.renderForceButton('READY', Status.READY)}
          {this.renderForceButton('STALE', Status.STALE)}
          {this.renderForceButton('ERROR', Status.ERROR)}
        </div>
        <WebPortal.ProfitAndLossController
          ref={this._controllerRef}
          currency={CAD}
          currencyDatabase={currencyDatabase}
          model={model}/>
      </div>);
  }

  private renderBehaviorButton(label: string,
      behavior: TestBehavior): JSX.Element {
    const isActive = this.state.behavior === behavior;
    return (
      <button key={label}
        style={{...STYLE.button, ...(isActive && STYLE.buttonActive)}}
        onClick={() => {
          model.behavior = behavior;
          this.setState({behavior});
        }}>
        {label}
      </button>);
  }

  private renderForceButton(label: string, status: Status): JSX.Element {
    return (
      <button key={label} style={STYLE.button}
        onClick={() => {
          this._controllerRef.current?.setState({
            status,
            report: status === Status.NONE ? null : REPORT
          } as any);
        }}>
        {label}
      </button>);
  }

  private _controllerRef:
    React.RefObject<WebPortal.ProfitAndLossController>;
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
  toolbarSeparator: {
    fontSize: '12px',
    color: '#999',
    padding: '0 4px'
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
