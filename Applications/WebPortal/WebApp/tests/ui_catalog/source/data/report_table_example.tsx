import * as Beam from 'beam';
import * as React from 'react';
import { ReportTable, SortableTableHeaderCell } from 'web_portal';

interface Properties {

  /** The number of sample reports to display. */
  reportCount: number;

  /** Whether to display the loading state. */
  loading: boolean;

  /** The selected report identifiers. */
  selected: string[];

  /** The column used to sort the sample reports. */
  sortColumn: ReportTable.Column;

  /** The order used to sort the sample reports. */
  sortOrder: SortableTableHeaderCell.SortOrder;

  /** Reports changes to the selected identifiers. */
  onSelectionChange?: (selected: string[]) => void;

  /** Reports requested changes to the sort order. */
  onSort?: (column: ReportTable.Column,
    order: SortableTableHeaderCell.SortOrder) => void;

  /** Reports navigation without leaving the catalog. */
  onNavigate?: (url: string) => void;
}

/** Demonstrates report selection, sorting, and loading. */
export class ReportTableExample extends React.Component<Properties> {
  public render(): JSX.Element {
    const reports = Array.from({length: this.props.reportCount}, (_, index) => {
      const types = ['Profit and Loss', 'Trading Volume',
        'Entitlement Activity'];
      const groups = ['Alpha Group', 'Gamma Group', 'Beta Group'];
      return {
        id: String(index + 1),
        type: types[index % types.length],
        parameters: ['Canada', groups[index % groups.length], 'Month to Date'],
        url: `/reports/${index + 1}`,
        dateCreated: new Beam.DateTime(new Beam.Date(2026, 9, 30 - index % 30))
      };
    });
    if(this.props.sortOrder !== SortableTableHeaderCell.SortOrder.NONE) {
      reports.sort((left, right) => {
        const result = (() => {
          switch(this.props.sortColumn) {
            case ReportTable.Column.TYPE:
              return left.type.localeCompare(right.type);
            case ReportTable.Column.PARAMETERS:
              return left.parameters.join(' ').localeCompare(
                right.parameters.join(' '));
            case ReportTable.Column.DATE_CREATED:
              return left.dateCreated.compare(right.dateCreated);
          }
        })();
        if(this.props.sortOrder ===
            SortableTableHeaderCell.SortOrder.DESCENDING) {
          return -result;
        }
        return result;
      });
    }
    return <ReportTable reports={reports}
      selected={new Set(this.props.selected)} loading={this.props.loading}
      sortColumn={this.props.sortColumn} sortOrder={this.props.sortOrder}
      onSelectionChange={this.onSelectionChange} onSort={this.props.onSort}
      onClick={this.onClick}/>;
  }

  private onSelectionChange = (selected: ReadonlySet<string>) => {
    this.props.onSelectionChange?.([...selected]);
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as HTMLElement).closest('a');
    if(link) {
      event.preventDefault();
      this.props.onNavigate?.(link.getAttribute('href'));
    }
  };
}
