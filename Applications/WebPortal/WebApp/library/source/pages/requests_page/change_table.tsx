import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DiffBadge } from './diff_badge';
import { EntitlementsStatusTag } from './entitlements_status_tag';

interface Properties {

  /** The list of entitlement changes to display. */
  changes: ChangeTable.Change[];
}

/** Displays a table of changes for an account modification request. */
export function ChangeTable(props: Properties) {
  return <FlatChangeTable changes={props.changes}/>;
}

interface FlatChangeTableProperties {

  /** The list of entitlement changes to display. */
  changes: ChangeTable.Change[];
}

interface FlatChangeTableState {
  isWide: boolean;
}

/** Displays a flat table of entitlement changes. */
class FlatChangeTable extends
    React.Component<FlatChangeTableProperties, FlatChangeTableState> {
  constructor(props: FlatChangeTableProperties) {
    super(props);
    this.containerRef = React.createRef<HTMLDivElement>();
    this.state = {
      isWide: false
    };
  }

  public componentDidMount(): void {
    this.resizeObserver = new ResizeObserver(this.onResize);
    if(this.containerRef.current) {
      this.resizeObserver.observe(this.containerRef.current);
    }
  }

  public componentWillUnmount(): void {
    this.resizeObserver?.disconnect();
  }

  public render(): JSX.Element {
    return (
      <div ref={this.containerRef} className={css(STYLES.container)}>
        <table className={css(STYLES.table,
            this.state.isWide ? STYLES.tableWide : STYLES.tableNarrow)}>
          {this.state.isWide &&
            <thead>
              <tr>
                <th className={css(STYLES.th)}>Property</th>
                <th className={css(STYLES.th)}>Current</th>
                <th className={css(STYLES.th)}>Request</th>
                <th className={css(STYLES.th)}>Change</th>
              </tr>
            </thead>}
          <tbody className={css(!this.state.isWide && STYLES.tbodyNarrow)}>
            {this.props.changes.map((change, index) =>
              <FlatTableRow key={index} change={change}
                isWide={this.state.isWide}/>)}
          </tbody>
        </table>
      </div>);
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    for(const entry of entries) {
      const isWide = entry.contentRect.width >= 544;
      if(isWide !== this.state.isWide) {
        this.setState({isWide});
      }
    }
  };

  private containerRef: React.RefObject<HTMLDivElement>;
  private resizeObserver?: ResizeObserver;
}

interface FlatTableRowProperties {

  /** The entitlement change to display. */
  change: ChangeTable.Change;

  /** Whether the table is in wide layout mode. */
  isWide: boolean;
}

/** Renders a single row in the flat change table. */
function FlatTableRow(props: FlatTableRowProperties) {
  if(props.isWide) {
    return (
      <tr className={css(STYLES.rowWide)}>
        <FlatTablePropertyCell name={props.change.name} isWide={true}/>
        <td className={css(STYLES.td)}>
          <EntitlementsStatusTag status={props.change.oldStatus}/>
        </td>
        <td className={css(STYLES.td)}>
          <EntitlementsStatusTag status={props.change.newStatus}/>
        </td>
        <td className={css(STYLES.td)}>
          <DiffBadge
            value={props.change.delta.value}
            direction={props.change.delta.direction}/>
        </td>
      </tr>);
  }
  return (
    <tr className={css(STYLES.rowNarrow)}>
      <FlatTablePropertyCell name={props.change.name} isWide={false}/>
      <td className={css(STYLES.tdNarrow)}>
        <span className={css(STYLES.valueLabel)}>Current</span>
        <EntitlementsStatusTag status={props.change.oldStatus}/>
      </td>
      <td className={css(STYLES.tdNarrow)}>
        <span className={css(STYLES.valueLabel)}>Request</span>
        <EntitlementsStatusTag status={props.change.newStatus}/>
      </td>
      <td className={css(STYLES.tdNarrow, STYLES.diffCellNarrow)}>
        <DiffBadge
          value={props.change.delta.value}
          direction={props.change.delta.direction}/>
      </td>
    </tr>);
}

interface FlatTablePropertyCellProperties {

  /** The name of the entitlement. */
  name: string;

  /** Whether the table is in wide layout mode. */
  isWide: boolean;
}

/** Renders the property name cell in a flat table row. */
function FlatTablePropertyCell(props: FlatTablePropertyCellProperties) {
  return (
    <td aria-label='Property' className={css(
        props.isWide ? STYLES.propertyCellWide : STYLES.propertyCellNarrow)}>
      {props.name}
    </td>);
}

export namespace ChangeTable {

  /** A change to a market data entitlement. */
  export interface Change {

    /** The name of the entitlement. */
    name: string;

    /** The current status. */
    oldStatus: EntitlementsStatusTag.Status;

    /** The requested status. */
    newStatus: EntitlementsStatusTag.Status;

    /** The associated fee delta. */
    delta: Delta;
  }

  /** A change between two values. */
  export interface Delta {

    /** The formatted delta value. */
    value: string;

    /** The direction of the change. */
    direction: DiffBadge.Direction;
  }
}

const STYLES = StyleSheet.create({
  container: {
    fontFamily: '"Roboto", system-ui, sans-serif'
  },
  table: {
    width: '100%',
    borderCollapse: 'separate' as 'separate',
    borderSpacing: 0,
    tableLayout: 'fixed' as 'fixed',
    color: '#333333',
    fontSize: '0.875rem'
  },
  tableWide: {
    borderRadius: '1px',
    border: '1px solid #E6E6E6'
  },
  tableNarrow: {
    display: 'block' as 'block'
  },
  tbodyNarrow: {
    display: 'block' as 'block'
  },
  th: {
    padding: '8px 19px',
    color: '#5D5E6D',
    fontWeight: 500,
    backgroundColor: '#F8F8F8',
    textAlign: 'left' as 'left'
  },
  td: {
    padding: '8px 19px'
  },
  tdNarrow: {
    display: 'block' as 'block',
    padding: '8px 19px'
  },
  rowWide: {
    backgroundColor: '#FFFFFF',
    ':hover': {
      backgroundColor: '#F8F8F8'
    }
  },
  rowNarrow: {
    display: 'block' as 'block',
    backgroundColor: '#FFFFFF',
    border: '1px solid #E6E6E6',
    borderRadius: '1px',
    marginBottom: '20px',
    ':last-child': {
      marginBottom: 0
    }
  },
  propertyCellWide: {
    padding: '8px 19px',
    fontWeight: 400,
    color: '#333333'
  },
  propertyCellNarrow: {
    display: 'block' as 'block',
    padding: '8px 19px',
    fontWeight: 500,
    color: '#5D5E6D',
    backgroundColor: '#F8F8F8'
  },
  valueLabel: {
    display: 'inline-block',
    width: '105px'
  },
  diffCellNarrow: {
    paddingLeft: '124px'
  }
});
