import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Disclosure, DropDownButton } from '../../..';
import { ProfitAndLossItem } from
  '../../../pages/account_page/profit_and_loss_page';
import { GroupProfitAndLossPage } from './group_profit_and_loss_page';

interface Properties {

  /** The username for the account. */
  username: string;

  /** The account currency symbol. */
  symbol: string;

  /** The account currency code. */
  code: string;

  /** The total profit and loss in account currency. */
  totalPnl: string;

  /** The list of currencies with activity. */
  currencies: GroupProfitAndLossPage.CurrencyEntry[];
}

interface State {
  isOpen: boolean;
}

/** Displays an expandable account entry within the group P&L report. */
export class AccountProfitAndLossItem extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {isOpen: false};
  }

  public render(): JSX.Element {
    const pnlText = (() => {
      const {symbol, totalPnl, code} = this.props;
      if(totalPnl.startsWith('-')) {
        return `-${symbol}${totalPnl.substring(1)} ${code}`;
      }
      return `${symbol}${totalPnl} ${code}`;
    })();
    const header = (
      <AccountHeader
        username={this.props.username}
        pnlText={pnlText}
        isOpen={this.state.isOpen}/>);
    const details = (
      <CurrencyList
        currencies={this.props.currencies}/>);
    return (
      <Disclosure open={this.state.isOpen}
        header={header} details={details} onToggle={this.onToggle}/>);
  }

  private onToggle = (isOpen: boolean) => {
    this.setState({isOpen});
  };
}

function AccountHeader(props: {
      username: string;
      pnlText: string;
      isOpen: boolean;
    }): JSX.Element {
  return (
    <div className={css(STYLES.header)}>
      <DropDownButton size='20' isExpanded={props.isOpen}/>
      <h3 className={css(STYLES.username)}
          style={
            props.isOpen ? {color: '#4B23A0', fontWeight: 500} : undefined}>
        {props.username}
      </h3>
      <span className={css(STYLES.totalPnl)}
          style={{color: '#36BB55', ...(props.isOpen && {fontWeight: 500})}}>
        {props.pnlText}
      </span>
    </div>);
}

function CurrencyList(props: {
      currencies: GroupProfitAndLossPage.CurrencyEntry[];
    }): JSX.Element {
  return (
    <ul className={css(STYLES.currencyList)}>
      {props.currencies.map((currency, i) =>
        <li key={i}>
          <ProfitAndLossItem
            symbol={currency.symbol}
            code={currency.code}
            totalProfitAndLoss={currency.totalProfitAndLoss}
            totalVolume={currency.totalVolume}
            totalFees={currency.totalFees}
            tickers={currency.tickers}/>
        </li>)}
    </ul>);
}

const STYLES = StyleSheet.create({
  header: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px',
    width: '100%',
    font: 'inherit',
    textAlign: 'start',
    padding: '5px 0',
    '@container (min-width: 656px)': {
      padding: '10px 0'
    }
  },
  username: {
    flex: 1,
    fontSize: '0.875rem',
    fontWeight: 'inherit',
    color: '#333333',
    margin: 0,
    padding: 0
  },
  totalPnl: {
    fontSize: '0.875rem',
    textAlign: 'end',
    whiteSpace: 'nowrap'
  },
  currencyList: {
    borderTop: '1px solid #E6E6E6',
    padding: '0 8px',
    listStyle: 'none',
    containerType: 'inline-size',
    '@container (min-width: 656px)': {
      paddingInline: '38px 0'
    }
  }
});
