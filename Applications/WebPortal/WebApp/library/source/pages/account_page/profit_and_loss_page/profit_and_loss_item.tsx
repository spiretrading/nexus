import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { DropDownButton } from '../../..';
import { ProfitAndLossTable } from './profit_and_loss_table';

interface Properties {

  /** The currency symbol (e.g. "$"). */
  symbol: string;

  /** The currency code (e.g. "CAD"). */
  code: string;

  /** The total profit and loss. */
  totalPnl: string;

  /** The total volume traded. */
  totalVolume: string;

  /** The total fees incurred. */
  totalFees: string;

  /** The list of securities traded. */
  securities: ProfitAndLossTable.Security[];
}

/** Displays an expandable item for a single currency's P&L. */
export function ProfitAndLossItem(props: Properties) {
  const [isExpanded, setIsExpanded] = React.useState(false);
  const pnl = parseFloat(props.totalPnl);
  const isNegative = pnl < 0;
  const pnlText = isNegative ?
    `-${props.symbol}${Math.abs(pnl)} ${props.code}` :
    `${props.symbol}${props.totalPnl} ${props.code}`;
  return (
    <div className={css(STYLES.container)}>
      <button className={css(STYLES.header)} type='button'
          onClick={() => setIsExpanded(!isExpanded)}>
        <DropDownButton size='20' isExpanded={isExpanded}
          onClick={() => setIsExpanded(!isExpanded)}/>
        <h2 className={css(STYLES.code)}
          style={isExpanded ?
            {color: '#4B23A0', fontWeight: 500} : undefined}>
          {props.code}
        </h2>
        <span className={css(STYLES.totalPnl)}
          style={{color: '#36BB55',
            ...(isExpanded && {fontWeight: 500})}}>
          {pnlText}
        </span>
      </button>
      {isExpanded &&
        <div className={css(STYLES.content)}>
          <ProfitAndLossTable symbol={props.symbol}
            totalPnl={props.totalPnl} totalVolume={props.totalVolume}
            totalFees={props.totalFees} securities={props.securities}/>
        </div>}
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
    padding: 0,
    border: 'none',
    background: 'none',
    cursor: 'pointer',
    font: 'inherit',
    textAlign: 'start'
  },
  code: {
    flex: 1,
    fontSize: 'inherit',
    fontWeight: 'inherit',
    color: '#333333',
    margin: 0
  },
  totalPnl: {
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
