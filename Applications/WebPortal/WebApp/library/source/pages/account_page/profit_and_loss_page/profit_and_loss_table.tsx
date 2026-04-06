import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { TableHeaderCell } from './table_header_cell';

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

interface State {
  sortColumn: ProfitAndLossTable.Column;
  sortOrder: TableHeaderCell.SortOrder;
}

/** Displays a table of securities with volume, fees, and P/L. */
export class ProfitAndLossTable extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      sortColumn: ProfitAndLossTable.Column.SECURITY,
      sortOrder: TableHeaderCell.SortOrder.NONE
    };
  }

  public render(): JSX.Element {
    return (
      <div className={css(STYLES.container)}>
        <table className={css(STYLES.table)}>
          <thead className={css(STYLES.thead)}>
            <tr>
              <TableHeaderCell
                  style={{textAlign: 'start'}}
                  sortOrder={this.sortOrderFor(
                    ProfitAndLossTable.Column.SECURITY)}
                  onSort={this.onSort(ProfitAndLossTable.Column.SECURITY)}>
                Security
              </TableHeaderCell>
              <TableHeaderCell
                  className={css(STYLES.collapsible)}
                  style={{textAlign: 'end'}}
                  sortOrder={this.sortOrderFor(
                    ProfitAndLossTable.Column.VOLUME)}
                  onSort={this.onSort(ProfitAndLossTable.Column.VOLUME)}>
                Volume
              </TableHeaderCell>
              <TableHeaderCell
                  className={css(STYLES.collapsible)}
                  style={{textAlign: 'end'}}
                  sortOrder={this.sortOrderFor(
                    ProfitAndLossTable.Column.FEES)}
                  onSort={this.onSort(ProfitAndLossTable.Column.FEES)}>
                Fees
              </TableHeaderCell>
              <TableHeaderCell
                  style={{textAlign: 'end'}}
                  sortOrder={this.sortOrderFor(
                    ProfitAndLossTable.Column.PNL)}
                  onSort={this.onSort(ProfitAndLossTable.Column.PNL)}>
                P/L
              </TableHeaderCell>
            </tr>
          </thead>
          <tbody>
            {this.props.securities.map((security, i) => {
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
                    {`${this.props.symbol}${security.fees}`}
                  </td>
                  <td className={css(STYLES.td)} aria-label='Profit and Loss'
                    style={{color: isNegative ? '#E63F44' : '#36BB55'}}>
                    {isNegative ?
                      `-${this.props.symbol}${security.pnl.replace('-', '')}` :
                      `${this.props.symbol}${security.pnl}`}
                  </td>
                </tr>);
            })}
            <tr className={css(STYLES.summaryRow)}>
              <td className={css(STYLES.td, STYLES.summaryCell, STYLES.tdStart)}>
                {`(${this.props.securities.length})`}
              </td>
              <td className={css(STYLES.td, STYLES.summaryCell,
                  STYLES.collapsible)}>
                {this.props.totalVolume}
              </td>
              <td className={css(STYLES.td, STYLES.summaryCell,
                  STYLES.collapsible)}>
                {`${this.props.symbol}${this.props.totalFees}`}
              </td>
              {(() => {
                const totalPnl = parseFloat(this.props.totalPnl);
                const isNegative = totalPnl < 0;
                return (
                  <td className={css(STYLES.td, STYLES.summaryCell)}
                    style={{color: isNegative ? '#E63F44' : '#36BB55'}}>
                    {isNegative ?
                      `-${this.props.symbol}${
                        this.props.totalPnl.replace('-', '')}` :
                      `${this.props.symbol}${this.props.totalPnl}`}
                  </td>);
              })()}
            </tr>
          </tbody>
        </table>
      </div>);
  }

  private sortOrderFor(
      column: ProfitAndLossTable.Column): TableHeaderCell.SortOrder {
    if(this.state.sortColumn === column) {
      return this.state.sortOrder;
    }
    return TableHeaderCell.SortOrder.NONE;
  }

  private onSort = (column: ProfitAndLossTable.Column) =>
      (order: TableHeaderCell.SortOrder) => {
    this.setState({sortColumn: column, sortOrder: order});
  }
}

export namespace ProfitAndLossTable {

  /** The columns that can be sorted. */
  export enum Column {
    SECURITY,
    VOLUME,
    FEES,
    PNL
  }

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
    width: '100%',
    containerType: 'inline-size',
    '@container (min-width: 424px)': {
      maxWidth: '528px'
    }
  },
  table: {
    width: '100%',
    color: '#333333',
    fontSize: '0.875rem',
    fontFamily: "'Roboto', system-ui, sans-serif",
    borderCollapse: 'separate',
    borderSpacing: 0,
    borderRadius: '1px',
    borderWidth: '1px',
    borderStyle: 'solid',
    borderColor: '#E6E6E6',
    tableLayout: 'fixed',
    '@container (min-width: 424px)': {
      tableLayout: 'auto'
    }
  },
  thead: {
    display: 'none',
    '@container (min-width: 424px)': {
      display: 'table-header-group'
    }
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
