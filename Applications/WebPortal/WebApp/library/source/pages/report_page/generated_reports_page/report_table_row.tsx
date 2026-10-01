import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { Checkbox, Link } from '../../../components';

interface Properties extends Omit<
    React.HTMLAttributes<HTMLTableRowElement>, 'id' | 'children' | 'onSelect'> {

  /** The report's identifier. */
  id: string;

  /** The report type displayed as a link. */
  type: string;

  /** The report's parameter values, in display order. */
  parameters: readonly string[];

  /** The link to the generated report. */
  url: string;

  /** The calendar date on which the report was generated. */
  dateCreated: Beam.Date;

  /** Whether the report is selected. */
  selected: boolean;

  /** Called when the checkbox changes.
   * @param id - The report's identifier.
   * @param selected - Whether the report should be selected.
   */
  onSelect?: (id: string, selected: boolean) => void;
}

/** Displays a generated report inside a table with controlled selection.
 * The table's containing element must establish an inline-size container.
 */
export class ReportTableRow extends React.Component<Properties> {
  public render(): JSX.Element {
    const {id, type, parameters, url, dateCreated, selected, onSelect,
      className, ...attributes} = this.props;
    return <tr {...attributes} aria-selected={selected}
        className={[css(STYLES.row), className].join(' ')}>
      <Selected type={type} selected={selected} onSelect={this.onSelect}/>
      <TypeCell type={type} url={url}/>
      <ParametersCell parameters={parameters}/>
      <DateCell date={dateCreated}/>
    </tr>;
  }

  private onSelect = (selected: boolean) => {
    this.props.onSelect?.(this.props.id, selected);
  };
}

function Selected(props: {type: string; selected: boolean;
    onSelect: (selected: boolean) => void}): JSX.Element {
  return <td className={css(STYLES.selected)}>
    <label>
      <span className={css(STYLES.hidden)}>Select {props.type} report</span>
      <Checkbox checked={props.selected} onClick={props.onSelect}/>
    </label>
  </td>;
}

function TypeCell(props: {type: string; url: string}): JSX.Element {
  return <td className={css(STYLES.cell, STYLES.type)}>
    <Link href={props.url} label={props.type}/>
  </td>;
}

function ParametersCell(props: {parameters: readonly string[]}): JSX.Element {
  return <td className={css(STYLES.cell)}>
    <div className={css(STYLES.content)}>
      <span className={css(STYLES.label)}>Parameters</span>
      <span className={css(STYLES.value)}>
        {props.parameters.join(' \u2022 ')}
      </span>
    </div>
  </td>;
}

function DateCell(props: {date: Beam.Date}): JSX.Element {
  const date = new Date(0);
  date.setFullYear(props.date.year, props.date.month - 1, props.date.day);
  const value = `${String(props.date.year).padStart(4, '0')}-` +
    `${String(props.date.month).padStart(2, '0')}-` +
    String(props.date.day).padStart(2, '0');
  return <td className={css(STYLES.cell)}>
    <div className={css(STYLES.content)}>
      <span className={css(STYLES.label)}>Date Created</span>
      <time dateTime={value} className={css(STYLES.value)}>
        {date.toLocaleDateString('en-US', {
          month: 'short', day: '2-digit', year: 'numeric'})}
      </time>
    </div>
  </td>;
}

const STYLES = StyleSheet.create({
  row: {
    boxSizing: 'border-box',
    backgroundColor: '#FFFFFF',
    color: '#333333',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontSize: '0.875rem',
    lineHeight: '16px',
    '@container (width <= 424px)': {
      display: 'grid',
      gridTemplateColumns: '48px minmax(0, 1fr)',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      ':is([aria-selected="true"])': {
        borderColor: '#684BC7',
        backgroundColor: '#E2E0FF'
      },
      ':is([aria-selected="true"]) > td': {
        backgroundColor: 'inherit'
      }
    },
    '@container (424px < width)': {
      border: 'none',
      borderRadius: 0,
      ':hover': {backgroundColor: '#F8F8F8'},
      ':is([aria-selected="true"])': {backgroundColor: '#E2E0FF'},
      ':is([aria-selected="true"]):hover': {backgroundColor: '#E2E0FF'}
    }
  },
  selected: {
    boxSizing: 'border-box',
    width: '48px',
    verticalAlign: 'top',
    '@container (width <= 424px)': {
      gridColumn: 1,
      gridRow: 1,
      padding: '10px 8px',
      backgroundColor: '#F8F8F8'
    },
    '@container (424px < width)': {
      padding: '10px 10px 10px 18px',
      backgroundColor: 'transparent'
    }
  },
  cell: {
    boxSizing: 'border-box',
    minHeight: '40px',
    verticalAlign: 'top',
    overflowWrap: 'anywhere',
    '@container (width <= 424px)': {
      gridColumn: 2,
      padding: '12px 20px 12px 10px'
    },
    '@container (424px < width)': {padding: '12px 20px'}
  },
  type: {
    '@container (width <= 424px)': {
      backgroundColor: '#F8F8F8',
      fontWeight: 500
    },
    '@container (424px < width)': {
      backgroundColor: 'transparent',
      fontWeight: 400
    }
  },
  content: {
    display: 'flex',
    '@container (width <= 424px)': {gap: '8px'}
  },
  label: {
    '@container (width <= 424px)': {flex: '0 0 40%'},
    '@container (424px < width)': {display: 'none'}
  },
  value: {minWidth: 0, flex: '1 1 0'},
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
