import * as Beam from 'beam';
import * as React from 'react';
import { ActivityTable, ReportActivityStatusTag, SortableTableHeaderCell } from
  'web_portal';

interface Properties {

  /** The number of sample activities to display. */
  activityCount: number;

  /** Whether to display the loading state. */
  loading: boolean;

  /** The selected report identifiers. */
  selected: string[];

  /** The column used to sort the sample activities. */
  sortColumn: ActivityTable.Column;

  /** The order used to sort the sample activities. */
  sortOrder: SortableTableHeaderCell.SortOrder;

  /** Reports changes to the selected identifiers. */
  onSelectionChange?: (selected: string[]) => void;

  /** Reports requested changes to the sort order. */
  onSort?: (column: ActivityTable.Column,
    order: SortableTableHeaderCell.SortOrder) => void;
}

/** Demonstrates report selection, sorting, and loading. */
export class ActivityTableExample extends React.Component<Properties> {
  public render(): JSX.Element {
    const activities = Array.from({length: this.props.activityCount},
        (_, index) => {
      const TYPES = ['Profit and Loss', 'Trading Volume',
        'Entitlement Activity'];
      const GROUPS = ['Alpha Group', 'Gamma Group', 'Beta Group'];
      return {
        id: String(index + 1),
        type: TYPES[index % TYPES.length],
        parameters: ['Canada', GROUPS[index % GROUPS.length], 'Month to Date'],
        status: (() => {
          if(index % 2 === 0) {
            return ReportActivityStatusTag.Status.GENERATING;
          }
          return ReportActivityStatusTag.Status.FAILED;
        })(),
        dateModified: new Beam.Date(2026, 9, 30 - index % 30)
      };
    });
    if(this.props.sortOrder !== SortableTableHeaderCell.SortOrder.NONE) {
      activities.sort((left, right) => {
        const result = (() => {
          switch(this.props.sortColumn) {
            case ActivityTable.Column.TYPE:
              return left.type.localeCompare(right.type);
            case ActivityTable.Column.PARAMETERS:
              return left.parameters.join(' ').localeCompare(
                right.parameters.join(' '));
            case ActivityTable.Column.STATUS:
              return left.status - right.status;
            case ActivityTable.Column.DATE_MODIFIED:
              return left.dateModified.compare(right.dateModified);
          }
        })();
        if(this.props.sortOrder ===
            SortableTableHeaderCell.SortOrder.DESCENDING) {
          return -result;
        }
        return result;
      });
    }
    return <ActivityTable activities={activities}
      selected={new Set(this.props.selected)} loading={this.props.loading}
      sortColumn={this.props.sortColumn} sortOrder={this.props.sortOrder}
      onSelectionChange={this.onSelectionChange} onSort={this.props.onSort}/>;
  }

  private onSelectionChange = (selected: ReadonlySet<string>) => {
    this.props.onSelectionChange?.([...selected]);
  };
}
