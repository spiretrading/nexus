import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { Checkbox, SortableTableHeaderCell } from '../../../components';
import { ReportTableRow } from './report_table_row';
import { ReportTableRowPlaceholder } from './report_table_row_placeholder';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'children' | 'onSelect'> {

  /** The reports to display, in their current sort order. */
  reports: readonly ReportTable.Report[];

  /** The identifiers of the selected reports. */
  selected: ReadonlySet<string>;

  /** Whether to display loading placeholders. Defaults to false. */
  loading?: boolean;

  /** The literal query to highlight in displayed report text. */
  highlight?: string;

  /** The column currently sorted. Defaults to no column. */
  sortColumn?: ReportTable.Column;

  /** The current sort order. Defaults to NONE. */
  sortOrder?: SortableTableHeaderCell.SortOrder;

  /** Called with the updated set of selected report identifiers.
   * Selection is cleared when the displayed report list changes.
   */
  onSelectionChange?: (selected: ReadonlySet<string>) => void;

  /** Called when a column's sort order is requested. */
  onSort?: (column: ReportTable.Column,
    order: SortableTableHeaderCell.SortOrder) => void;
}

/** Displays generated reports with selection and sorting controls. */
export class ReportTable extends React.Component<Properties> {
  public render(): JSX.Element {
    const {reports, selected, loading, highlight, sortColumn, sortOrder,
      onSelectionChange, onSort, className, ...attributes} = this.props;
    return <div {...attributes}
        className={[css(STYLES.container), className].join(' ')}>
      <Table reports={reports} selected={selected} loading={loading}
        highlight={highlight} sortColumn={sortColumn} sortOrder={sortOrder}
        onSort={onSort} onSelect={this.onSelect}
        onSelectAll={this.onSelectAll}/>
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(this.props.selected.size > 0 &&
        !areReportsEqual(previous.reports, this.props.reports)) {
      this.props.onSelectionChange?.(new Set<string>());
    }
  }

  private onSelect = (id: string, checked: boolean) => {
    if(this.props.loading) {
      return;
    }
    const selected = new Set(this.props.selected);
    if(checked) {
      selected.add(id);
    } else {
      selected.delete(id);
    }
    this.props.onSelectionChange?.(selected);
  };

  private onSelectAll = (checked: boolean) => {
    if(this.props.loading) {
      return;
    }
    const selected = new Set<string>();
    if(checked) {
      for(const id of this.props.selected) {
        selected.add(id);
      }
      for(const report of this.props.reports) {
        selected.add(report.id);
      }
    }
    this.props.onSelectionChange?.(selected);
  };
}

export namespace ReportTable {

  /** The sortable report columns. */
  export enum Column {

    /** The report type. */
    TYPE,

    /** The report's parameter values. */
    PARAMETERS,

    /** The date the report was generated. */
    DATE_CREATED
  }

  /** A generated report displayed in the table. */
  export interface Report {

    /** The report's unique identifier. */
    readonly id: string;

    /** The report type. */
    readonly type: string;

    /** The report's parameter values, in display order. */
    readonly parameters: readonly string[];

    /** The link to the page represented by the report. */
    readonly url: string;

    /** The UTC timestamp when the report was generated. */
    readonly dateCreated: Beam.DateTime;
  }
}

interface TableProperties extends Pick<Properties, 'reports' | 'selected' |
    'loading' | 'highlight' | 'sortColumn' | 'sortOrder' | 'onSort'> {
  onSelect: (id: string, selected: boolean) => void;
  onSelectAll: (checked: boolean) => void;
}

function Table(props: TableProperties): JSX.Element {
  return <table className={css(STYLES.table)} aria-busy={!!props.loading}>
    <colgroup className={css(STYLES.wide)}>
      <col style={{width: '48px'}}/>
      <col style={{width: 'min(25cqw, 180px)'}}/>
      <col/>
      <col style={{width: 'min(25cqw, 132px)'}}/>
    </colgroup>
    <Head {...props}/>
    <Body {...props}/>
  </table>;
}

function Head(props: TableProperties): JSX.Element {
  return <thead className={css(STYLES.head, STYLES.wide)}>
    <HeaderRow {...props}/>
  </thead>;
}

class HeaderRow extends React.Component<TableProperties> {
  public render(): JSX.Element {
    const checked = this.props.reports.length > 0 &&
      this.props.reports.every(report => this.props.selected.has(report.id));
    return <tr>
      <SelectAllCell checked={checked}
        indeterminate={!checked && this.props.selected.size > 0}
        disabled={this.props.loading} onSelect={this.props.onSelectAll}/>
      {['Type', 'Parameters', 'Date Created'].map((label, column) =>
        <SortableTableHeaderCell key={column} scope='col'
            disabled={this.props.loading}
            sortOrder={(() => {
              if(column === this.props.sortColumn) {
                return this.props.sortOrder;
              }
              return SortableTableHeaderCell.SortOrder.NONE;
            })()} onSort={this.onSort(column)}>
          {label}
        </SortableTableHeaderCell>)}
    </tr>;
  }

  private onSort = (column: ReportTable.Column) =>
      (order: SortableTableHeaderCell.SortOrder) => {
    if(!this.props.loading) {
      this.props.onSort?.(column, order);
    }
  };
}

interface SelectAllProperties {
  checked: boolean;
  indeterminate: boolean;
  disabled: boolean;
  onSelect: (checked: boolean) => void;
}

function SelectAllCell(props: SelectAllProperties): JSX.Element {
  return <th scope='col' className={css(STYLES.selection)}>
    <label>
      <span className={css(STYLES.hidden)}>Select all reports</span>
      <Checkbox checked={props.checked} indeterminate={props.indeterminate}
        disabled={props.disabled} onClick={props.onSelect}/>
    </label>
  </th>;
}

function Body(props: TableProperties): JSX.Element {
  return <tbody className={css(STYLES.body)}>
    {(() => {
      if(props.loading) {
        return Array.from({length: 5}, (_, index) =>
          <ReportTableRowPlaceholder key={index}/>);
      }
      return props.reports.map(report =>
        <ReportTableRow key={report.id} {...report}
          highlight={props.highlight}
          selected={props.selected.has(report.id)} onSelect={props.onSelect}/>);
    })()}
  </tbody>;
}

function areReportsEqual(left: readonly ReportTable.Report[],
    right: readonly ReportTable.Report[]): boolean {
  return left.length === right.length && left.every((report, index) => {
    const other = right[index];
    return report.id === other.id && report.type === other.type &&
      report.url === other.url &&
      report.dateCreated.compare(other.dateCreated) === 0 &&
      report.parameters.length === other.parameters.length &&
      report.parameters.every((value, index) =>
        value === other.parameters[index]);
  });
}

const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%'},
  table: {
    boxSizing: 'border-box',
    width: '100%',
    maxWidth: '696px',
    tableLayout: 'fixed',
    borderCollapse: 'separate',
    borderSpacing: 0,
    color: '#333333',
    fontSize: '0.875rem',
    fontFamily: 'Roboto, system-ui, sans-serif',
    '@container (width <= 424px)': {display: 'block'},
    '@container (424px < width)': {
      borderRadius: '1px',
      border: '1px solid #E6E6E6'
    }
  },
  wide: {'@container (width <= 424px)': {display: 'none'}},
  head: {backgroundColor: '#F8F8F8'},
  selection: {
    boxSizing: 'border-box',
    padding: '10px 10px 10px 18px',
    height: '40px'
  },
  body: {
    '@container (width <= 424px)': {
      display: 'grid',
      gap: '20px'
    }
  },
  hidden: {
    position: 'absolute',
    width: '1px',
    height: '1px',
    padding: 0,
    overflow: 'hidden',
    clipPath: 'inset(50%)',
    whiteSpace: 'nowrap'
  }
});
