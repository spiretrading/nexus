import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { Checkbox, SortableTableHeaderCell } from '../../../components';
import { ActivityTableRow } from './activity_table_row';
import { ActivityTableRowPlaceholder } from './activity_table_row_placeholder';
import { ReportActivityStatusTag } from './report_activity_status_tag';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'children' | 'onSelect'> {

  /** The activities to display, in their current sort order. */
  activities: readonly ActivityTable.Activity[];

  /** The identifiers of the selected activities. */
  selected: ReadonlySet<string>;

  /** Whether to display loading placeholders. Defaults to false. */
  loading?: boolean;

  /** The column currently sorted. Defaults to no column. */
  sortColumn?: ActivityTable.Column;

  /** The current sort order. Defaults to NONE. */
  sortOrder?: SortableTableHeaderCell.SortOrder;

  /** Called with the updated set of selected report identifiers.
   * Selection is cleared when the displayed report list changes.
   */
  onSelectionChange?: (selected: ReadonlySet<string>) => void;

  /** Called when a column's sort order is requested. */
  onSort?: (column: ActivityTable.Column,
    order: SortableTableHeaderCell.SortOrder) => void;
}

/** Displays report activity with selection and sorting controls. */
export class ActivityTable extends React.Component<Properties> {
  public render(): JSX.Element {
    const {activities, selected, loading, sortColumn, sortOrder,
      onSelectionChange, onSort, className, ...attributes} = this.props;
    return <div {...attributes}
        className={[css(STYLES.container), className].join(' ')}>
      <Table activities={activities} selected={selected} loading={loading}
        sortColumn={sortColumn} sortOrder={sortOrder}
        onSort={onSort} onSelect={this.onSelect}
        onSelectAll={this.onSelectAll}/>
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(this.props.selected.size > 0 &&
        !areActivitiesEqual(previous.activities, this.props.activities)) {
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
      for(const report of this.props.activities) {
        selected.add(report.id);
      }
    }
    this.props.onSelectionChange?.(selected);
  };
}

export namespace ActivityTable {

  /** The sortable report columns. */
  export enum Column {

    /** The report type. */
    TYPE,

    /** The report's parameter values. */
    PARAMETERS,

    /** The report's activity status. */
    STATUS,

    /** The date the activity was last updated. */
    DATE_MODIFIED
  }

  /** A report activity entry displayed in the table. */
  export interface Activity {

    /** The report's unique identifier. */
    readonly id: string;

    /** The report type. */
    readonly type: string;

    /** The report's parameter values, in display order. */
    readonly parameters: readonly string[];

    /** The report's activity status. */
    readonly status: ReportActivityStatusTag.Status;

    /** The calendar date on which the activity was last updated. */
    readonly dateModified: Beam.Date;
  }
}

interface TableProperties extends Pick<Properties, 'activities' | 'selected' |
    'loading' | 'sortColumn' | 'sortOrder' | 'onSort'> {
  onSelect: (id: string, selected: boolean) => void;
  onSelectAll: (checked: boolean) => void;
}

function Table(props: TableProperties): JSX.Element {
  return <table className={css(STYLES.table)} aria-busy={!!props.loading}>
    <colgroup className={css(STYLES.wide)}>
      <col style={{width: '48px'}}/>
      <col style={{width: 'min(20cqw, 180px)'}}/>
      <col/>
      <col style={{width: '100px'}}/>
      <col style={{width: 'min(20cqw, 132px)'}}/>
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
    const checked = this.props.activities.length > 0 &&
      this.props.activities.every(report => this.props.selected.has(report.id));
    return <tr>
      <SelectAllCell checked={checked}
        indeterminate={!checked && this.props.selected.size > 0}
        disabled={this.props.loading} onSelect={this.props.onSelectAll}/>
      {['Type', 'Parameters', 'Status', 'Last Modified'].map((label, column) =>
        <SortableTableHeaderCell key={column} scope='col'
            className={css(STYLES.headerCell)}
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

  private onSort = (column: ActivityTable.Column) =>
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
          <ActivityTableRowPlaceholder key={index}/>);
      }
      return props.activities.map(report =>
        <ActivityTableRow key={report.id} {...report}
          selected={props.selected.has(report.id)} onSelect={props.onSelect}/>);
    })()}
  </tbody>;
}

function areActivitiesEqual(left: readonly ActivityTable.Activity[],
    right: readonly ActivityTable.Activity[]): boolean {
  return left.length === right.length && left.every((report, index) => {
    const other = right[index];
    return report.id === other.id && report.type === other.type &&
      report.status === other.status &&
      report.dateModified.compare(other.dateModified) === 0 &&
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
    maxWidth: '796px',
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
  headerCell: {overflowWrap: 'anywhere'},
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
