import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The currency symbol (e.g. "$"). */
  symbol: string;

  /** The total profit and loss. */
  totalPnl: string;

  /** The total volume traded. */
  totalVolume: string;

  /** The total fees incurred. */
  totalFees: string;

  /** The list of securities traded. */
  securities: ProfitAndLossTable.Security[];
}

/** Displays a table of securities with volume, fees, and P/L. */
export function ProfitAndLossTable(props: Properties) {
  return (
    <div className={css(STYLES.container)}>
      <table className={css(STYLES.table)}>
        <thead className={css(STYLES.thead)}>
          <tr>
            <th className={css(STYLES.th, STYLES.thStart)}>Security</th>
            <th className={css(STYLES.th, STYLES.collapsible)}>Volume</th>
            <th className={css(STYLES.th, STYLES.collapsible)}>Fees</th>
            <th className={css(STYLES.th)}>P/L</th>
          </tr>
        </thead>
        <tbody>
          {props.securities.map((security, i) => {
            const pnl = parseFloat(security.pnl);
            const isNegative = pnl < 0;
            return (
              <tr key={i} className={css(STYLES.row)}>
                <td className={css(STYLES.td, STYLES.tdStart)}
                  aria-label='Security'>
                  {security.symbol}
                </td>
                <td className={css(STYLES.td, STYLES.collapsible)}
                  aria-label='Volume'>
                  {security.volume}
                </td>
                <td className={css(STYLES.td, STYLES.collapsible)}
                  aria-label='Fees'>
                  {`${props.symbol}${security.fees}`}
                </td>
                <td className={css(STYLES.td)} aria-label='Profit and Loss'
                  style={{color: isNegative ? '#E63F44' : '#36BB55'}}>
                  {isNegative ?
                    `-${props.symbol}${Math.abs(pnl)}` :
                    `${props.symbol}${security.pnl}`}
                </td>
              </tr>);
          })}
          <tr className={css(STYLES.summaryRow)}>
            <td className={css(STYLES.td, STYLES.summaryCell, STYLES.tdStart)}>
              {`(${props.securities.length})`}
            </td>
            <td className={css(STYLES.td, STYLES.summaryCell,
                STYLES.collapsible)}>
              {props.totalVolume}
            </td>
            <td className={css(STYLES.td, STYLES.summaryCell,
                STYLES.collapsible)}>
              {`${props.symbol}${props.totalFees}`}
            </td>
            {(() => {
              const totalPnl = parseFloat(props.totalPnl);
              const isNegative = totalPnl < 0;
              return (
                <td className={css(STYLES.td, STYLES.summaryCell)}
                  style={{color: isNegative ? '#E63F44' : '#36BB55'}}>
                  {isNegative ?
                    `-${props.symbol}${Math.abs(totalPnl)}` :
                    `${props.symbol}${props.totalPnl}`}
                </td>);
            })()}
          </tr>
        </tbody>
      </table>
    </div>);
}

export namespace ProfitAndLossTable {
  export interface Security {

    /** The symbol of the security. */
    symbol: string;

    /** The volume traded. */
    volume: string;

    /** The fees incurred. */
    fees: string;

    /** The profit and loss. */
    pnl: string;
  }
}

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size'
  },
  table: {
    width: '100%',
    color: '#333333',
    fontSize: '0.875rem',
    fontFamily: "'Roboto', system-ui, sans-serif",
    borderCollapse: 'separate',
    borderSpacing: 0,
    borderRadius: '1px',
    border: '1px solid #E6E6E6',
    tableLayout: 'fixed',
    '@container (min-width: 424px)': {
      width: '528px',
      tableLayout: 'auto' as any
    }
  },
  thead: {
    display: 'none',
    backgroundColor: '#F8F8F8',
    color: '#5D5E6D',
    fontWeight: 500,
    '@container (min-width: 424px)': {
      display: 'table-header-group'
    }
  },
  th: {
    padding: '12px 19px',
    textAlign: 'end',
    fontWeight: 'inherit'
  },
  thStart: {
    textAlign: 'start'
  },
  row: {
    backgroundColor: '#FFFFFF',
    '@container (min-width: 424px)': {
      ':hover': {
        backgroundColor: '#F8F8F8'
      }
    }
  },
  td: {
    padding: '12px 19px',
    textAlign: 'end'
  },
  tdStart: {
    textAlign: 'start'
  },
  collapsible: {
    display: 'none',
    '@container (min-width: 424px)': {
      display: 'table-cell'
    }
  },
  summaryRow: {
    display: 'none',
    '@container (min-width: 424px)': {
      display: 'table-row'
    }
  },
  summaryCell: {
    borderTop: '1px solid #E6E6E6',
    fontWeight: 500
  }
});
