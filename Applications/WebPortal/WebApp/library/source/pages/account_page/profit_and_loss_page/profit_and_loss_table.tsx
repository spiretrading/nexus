import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { TableHeaderCell } from './table_header_cell';

interface Properties {

  /** The currency symbol (e.g. "$"). */
  symbol: string;

  /** The total profit and loss. */
  totalProfitAndLoss: Nexus.Money;

  /** The total volume traded. */
  totalVolume: Nexus.Quantity;

  /** The total fees incurred. */
  totalFees: Nexus.Money;

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
    const sorted = this.sortedSecurities();
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
            {sorted.map((security, i) => {
              const isNegative = security.profitAndLoss.compare(
                Nexus.Money.ZERO) < 0;
              return (
                <tr key={i} className={css(STYLES.row)}>
                  <td className={css(STYLES.td, STYLES.tdStart)}
                    aria-label='Security'>
                    {security.security.toString()}
                  </td>
                  <td className={css(STYLES.td, STYLES.collapsible)}
                    aria-label='Volume'>
                    {security.volume.toString()}
                  </td>
                  <td className={css(STYLES.td, STYLES.collapsible)}
                    aria-label='Fees'>
                    {formatMoney(this.props.symbol, security.fees)}
                  </td>
                  <td className={css(STYLES.td)} aria-label='Profit and Loss'
                    style={{color: isNegative ? '#E63F44' : '#36BB55'}}>
                    {formatMoney(this.props.symbol, security.profitAndLoss)}
                  </td>
                </tr>);
            })}
            {(() => {
              const isNegative = this.props.totalProfitAndLoss.compare(
                Nexus.Money.ZERO) < 0;
              return (
                <tr className={css(STYLES.summaryRow)}>
                  <td className={css(STYLES.td, STYLES.summaryCell,
                      STYLES.tdStart)}>
                    {`(${this.props.securities.length})`}
                  </td>
                  <td className={css(STYLES.td, STYLES.summaryCell,
                      STYLES.collapsible)}>
                    {this.props.totalVolume.toString()}
                  </td>
                  <td className={css(STYLES.td, STYLES.summaryCell,
                      STYLES.collapsible)}>
                    {formatMoney(this.props.symbol, this.props.totalFees)}
                  </td>
                  <td className={css(STYLES.td, STYLES.summaryCell)}
                    style={{color: isNegative ? '#E63F44' : '#36BB55'}}>
                    {formatMoney(this.props.symbol,
                      this.props.totalProfitAndLoss)}
                  </td>
                </tr>);
            })()}
          </tbody>
        </table>
      </div>);
  }

  private sortedSecurities(): ProfitAndLossTable.Security[] {
    if(this.state.sortOrder === TableHeaderCell.SortOrder.NONE) {
      return this.props.securities;
    }
    var sorted = [...this.props.securities];
    var direction =
      this.state.sortOrder === TableHeaderCell.SortOrder.ASCENDING ? 1 : -1;
    sorted.sort((a, b) => {
      switch(this.state.sortColumn) {
        case ProfitAndLossTable.Column.SECURITY:
          return direction * a.security.toString().localeCompare(
            b.security.toString());
        case ProfitAndLossTable.Column.VOLUME:
          return direction * a.volume.compare(b.volume);
        case ProfitAndLossTable.Column.FEES:
          return direction * a.fees.compare(b.fees);
        case ProfitAndLossTable.Column.PNL:
          return direction * a.profitAndLoss.compare(b.profitAndLoss);
      }
    });
    return sorted;
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

function formatMoney(symbol: string, value: Nexus.Money): string {
  var text = value.toString();
  if(text.startsWith('-')) {
    return `-${symbol}${text.substring(1)}`;
  }
  return `${symbol}${text}`;
}

export namespace ProfitAndLossTable {

  /** The columns that can be sorted. */
  export enum Column {
    SECURITY,
    VOLUME,
    FEES,
    PNL
  }

  /** A row in the table. */
  export interface Security {

    /** The security traded. */
    security: Nexus.Security;

    /** The volume traded. */
    volume: Nexus.Quantity;

    /** The fees incurred. */
    fees: Nexus.Money;

    /** The profit and loss. */
    profitAndLoss: Nexus.Money;
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
