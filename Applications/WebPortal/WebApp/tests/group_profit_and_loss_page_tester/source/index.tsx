import * as Beam from 'beam';
import Fraction from 'fraction.js';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const currencyDatabase = Nexus.buildCurrencyDatabase();
const CAD = Nexus.Currencies.CAD;
const AUD = Nexus.Currencies.AUD;

const TSX = Nexus.Venues.TSX;
const ASX = Nexus.Venues.ASX;

const REPORT: WebPortal.GroupProfitAndLossModel.Report = {
  totalProfitAndLoss: Nexus.Money.parse('6537.15'),
  totalFees: Nexus.Money.parse('538.00'),
  totalVolume: Nexus.Quantity.parse('53800'),
  accounts: [
    {
      account: new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 1, 'trader_one'),
      totalProfitAndLoss: Nexus.Money.parse('4026.53'),
      currencies: [
        {
          currency: CAD,
          totalProfitAndLoss: Nexus.Money.parse('2635.68'),
          totalVolume: Nexus.Quantity.parse('25950'),
          totalFees: Nexus.Money.parse('259.50'),
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
              profitAndLoss: Nexus.Money.parse('891.33')}
          ]
        },
        {
          currency: AUD,
          totalProfitAndLoss: Nexus.Money.parse('1390.85'),
          totalVolume: Nexus.Quantity.parse('5500'),
          totalFees: Nexus.Money.parse('55.00'),
          tickers: [
            {ticker: new Nexus.Ticker('BHP', ASX),
              volume: Nexus.Quantity.parse('3400'),
              fees: Nexus.Money.parse('34.00'),
              profitAndLoss: Nexus.Money.parse('912.55')},
            {ticker: new Nexus.Ticker('CBA', ASX),
              volume: Nexus.Quantity.parse('2100'),
              fees: Nexus.Money.parse('21.00'),
              profitAndLoss: Nexus.Money.parse('478.30')}
          ]
        }
      ]
    },
    {
      account: new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 2, 'trader_two'),
      totalProfitAndLoss: Nexus.Money.parse('913.27'),
      currencies: [
        {
          currency: CAD,
          totalProfitAndLoss: Nexus.Money.parse('913.27'),
          totalVolume: Nexus.Quantity.parse('10850'),
          totalFees: Nexus.Money.parse('108.50'),
          tickers: [
            {ticker: new Nexus.Ticker('ENB', TSX),
              volume: Nexus.Quantity.parse('3100'),
              fees: Nexus.Money.parse('31.00'),
              profitAndLoss: Nexus.Money.parse('445.67')},
            {ticker: new Nexus.Ticker('CNR', TSX),
              volume: Nexus.Quantity.parse('2750'),
              fees: Nexus.Money.parse('27.50'),
              profitAndLoss: Nexus.Money.parse('-312.40')},
            {ticker: new Nexus.Ticker('SU', TSX),
              volume: Nexus.Quantity.parse('5000'),
              fees: Nexus.Money.parse('50.00'),
              profitAndLoss: Nexus.Money.parse('780.00')}
          ]
        }
      ]
    },
    {
      account: new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 3, 'trader_three'),
      totalProfitAndLoss: Nexus.Money.parse('1597.35'),
      currencies: [
        {
          currency: AUD,
          totalProfitAndLoss: Nexus.Money.parse('662.85'),
          totalVolume: Nexus.Quantity.parse('4500'),
          totalFees: Nexus.Money.parse('45.00'),
          tickers: [
            {ticker: new Nexus.Ticker('CSL', ASX),
              volume: Nexus.Quantity.parse('1800'),
              fees: Nexus.Money.parse('18.00'),
              profitAndLoss: Nexus.Money.parse('540.20')},
            {ticker: new Nexus.Ticker('NAB', ASX),
              volume: Nexus.Quantity.parse('1500'),
              fees: Nexus.Money.parse('15.00'),
              profitAndLoss: Nexus.Money.parse('-210.45')},
            {ticker: new Nexus.Ticker('WES', ASX),
              volume: Nexus.Quantity.parse('1200'),
              fees: Nexus.Money.parse('12.00'),
              profitAndLoss: Nexus.Money.parse('333.10')}
          ]
        },
        {
          currency: CAD,
          totalProfitAndLoss: Nexus.Money.parse('934.50'),
          totalVolume: Nexus.Quantity.parse('7000'),
          totalFees: Nexus.Money.parse('70.00'),
          tickers: [
            {ticker: new Nexus.Ticker('CP', TSX),
              volume: Nexus.Quantity.parse('4500'),
              fees: Nexus.Money.parse('45.00'),
              profitAndLoss: Nexus.Money.parse('320.00')},
            {ticker: new Nexus.Ticker('SHOP', TSX),
              volume: Nexus.Quantity.parse('2500'),
              fees: Nexus.Money.parse('25.00'),
              profitAndLoss: Nexus.Money.parse('614.50')}
          ]
        }
      ]
    }
  ],
  exchangeRates: [
    new Nexus.ExchangeRate(
      new Nexus.CurrencyPair(AUD, CAD), new Fraction(88, 100))
  ]
};

enum TestBehavior {
  SUCCEED,
  FAIL,
  HANG
}

class TestModel extends WebPortal.GroupProfitAndLossModel {
  constructor(report: WebPortal.GroupProfitAndLossModel.Report, delay: number) {
    super();
    this.report = report;
    this.delay = delay;
    this.nextId = 1;
    this.pending = new Map();
    this.behavior = TestBehavior.SUCCEED;
  }

  public setBehavior(behavior: TestBehavior): void {
    this.behavior = behavior;
  }

  public async load(): Promise<void> {
    return;
  }

  public async startReport(_start: Beam.Date, _end: Beam.Date):
      Promise<number> {
    const id = this.nextId++;
    this.pending.set(id, {reject: null});
    return id;
  }

  public async awaitReport(id: number):
      Promise<WebPortal.GroupProfitAndLossModel.Report> {
    const entry = this.pending.get(id);
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
      }, this.delay);
      entry.reject = () => {
        clearTimeout(timer);
        reject(new Error('Report cancelled.'));
      };
    });
    this.pending.delete(id);
    return this.report;
  }

  public async cancelReport(id: number): Promise<void> {
    const entry = this.pending.get(id);
    if(entry) {
      entry.reject?.();
      this.pending.delete(id);
    }
  }

  private report: WebPortal.GroupProfitAndLossModel.Report;
  private delay: number;
  private nextId: number;
  private pending: Map<number, {reject: () => void}>;
  private behavior: TestBehavior;
}

const model = new TestModel(REPORT, 2000);

interface State {
  behavior: TestBehavior;
}

class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      behavior: TestBehavior.SUCCEED
    };
  }

  public render(): JSX.Element {
    return (
      <div style={STYLE.wrapper}>
        <div style={STYLE.toolbar}>
          <span style={STYLE.toolbarLabel}>Behavior:</span>
          {this.renderBehaviorButton('Succeed', TestBehavior.SUCCEED)}
          {this.renderBehaviorButton('Fail', TestBehavior.FAIL)}
          {this.renderBehaviorButton('Hang', TestBehavior.HANG)}
        </div>
        <WebPortal.GroupProfitAndLossController
          currency={CAD}
          currencyDatabase={currencyDatabase}
          model={model}/>
      </div>);
  }

  private renderBehaviorButton(
      label: string, behavior: TestBehavior): JSX.Element {
    const isActive = this.state.behavior === behavior;
    return (
      <button key={label}
        style={{...STYLE.button, ...(isActive && STYLE.buttonActive)}}
        onClick={() => {
          model.setBehavior(behavior);
          this.setState({behavior});
        }}>
        {label}
      </button>);
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
