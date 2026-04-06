import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Disclosure, DropDownButton } from '../../..';
import { ProfitAndLossTable } from './profit_and_loss_table';

interface Properties {

  /** The currency symbol (e.g. "$"). */
  symbol: string;

  /** The currency code (e.g. "CAD"). */
  code: string;

  /** The total profit and loss. */
  totalProfitAndLoss: Nexus.Money;

  /** The total volume traded. */
  totalVolume: Nexus.Quantity;

  /** The total fees incurred. */
  totalFees: Nexus.Money;

  /** The list of securities traded. */
  securities: ProfitAndLossTable.Security[];
}

/** Displays an expandable item for a single currency's P&L. */
export function ProfitAndLossItem(props: Properties) {
  const [isOpen, setIsOpen] = React.useState(false);
  const isNegative = props.totalProfitAndLoss.compare(Nexus.Money.ZERO) < 0;
  const pnlText = (() => {
    var text = props.totalProfitAndLoss.toString();
    if(isNegative) {
      return `-${props.symbol}${text.substring(1)} ${props.code}`;
    }
    return `${props.symbol}${text} ${props.code}`;
  })();
  const header = (
    <div className={css(STYLES.header)}>
      <DropDownButton size='20' isExpanded={isOpen}/>
      <h2 className={css(STYLES.code)}
        style={isOpen ?
          {color: '#4B23A0', fontWeight: 500} : undefined}>
        {props.code}
      </h2>
      <span className={css(STYLES.totalPnl)}
        style={{color: '#36BB55',
          ...(isOpen && {fontWeight: 500})}}>
        {pnlText}
      </span>
    </div>);
  const details = (
    <div className={css(STYLES.content)}>
      <ProfitAndLossTable symbol={props.symbol}
        totalProfitAndLoss={props.totalProfitAndLoss}
        totalVolume={props.totalVolume}
        totalFees={props.totalFees} securities={props.securities}/>
    </div>);
  return (
    <div className={css(STYLES.container)}>
      <Disclosure open={isOpen} header={header}
        details={details} onToggle={setIsOpen}/>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size'
  },
  header: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px',
    width: '100%',
    height: '40px',
    font: 'inherit',
    textAlign: 'start'
  },
  code: {
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
  content: {
    borderTop: '1px solid #E6E6E6',
    padding: '20px 0',
    '@container (min-width: 732px)': {
      paddingInlineStart: '38px'
    }
  }
});
