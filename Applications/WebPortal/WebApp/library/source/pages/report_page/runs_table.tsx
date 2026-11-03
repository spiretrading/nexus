import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as React from 'react';
import { formatReportDate } from './generated_reports_page/report_date';

interface Properties extends
    Omit<React.TableHTMLAttributes<HTMLTableElement>, 'children'> {

  /** Upcoming run times in display order, using the schedule's timezone. */
  runs: readonly Beam.DateTime[];
}

/** Displays the upcoming dates and times of a repeating report. */
export function RunsTable(props: Properties): JSX.Element {
  const {runs, className, ...attributes} = props;
  return <table {...attributes}
      className={[css(STYLES.table), className].join(' ')}>
    <TableHeader/>
    <TableBody runs={runs}/>
  </table>;
}

function TableHeader(): JSX.Element {
  return <thead className={css(STYLES.header)}><HeaderRow/></thead>;
}

function HeaderRow(): JSX.Element {
  return <tr><HeaderCell/></tr>;
}

function HeaderCell(): JSX.Element {
  return <th scope='col' className={css(STYLES.cell, STYLES.heading)}>
    <div className={css(STYLES.content)}>Upcoming Runs</div>
  </th>;
}

function TableBody(props: {runs: readonly Beam.DateTime[]}): JSX.Element {
  return <tbody>
    {props.runs.map((run, index) => <RunRow key={index} run={run}/>)}
  </tbody>;
}

function RunRow(props: {run: Beam.DateTime}): JSX.Element {
  return <tr className={css(STYLES.row)}><RunCell run={props.run}/></tr>;
}

function RunCell(props: {run: Beam.DateTime}): JSX.Element {
  const time = props.run.timeOfDay.split();
  const period = (() => {
    if(time.hours < 12) {
      return 'AM';
    }
    return 'PM';
  })();
  const hours = time.hours % 12 || 12;
  const minutes = String(time.minutes).padStart(2, '0');
  const seconds = String(time.seconds).padStart(2, '0');
  return <td className={css(STYLES.cell)}>
    <div className={css(STYLES.content)}>
      {formatReportDate(props.run.date)} at {hours}:{minutes}:{seconds} {period}
    </div>
  </td>;
}

const STYLES = StyleSheet.create({
  table: {
    width: '100%',
    boxSizing: 'border-box',
    borderCollapse: 'separate',
    borderSpacing: 0,
    border: '1px solid #E6E6E6',
    borderRadius: '1px',
    color: '#333333',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontSize: '0.875rem',
    fontWeight: 400,
    textAlign: 'left',
    tableLayout: 'fixed'
  },
  header: {
    backgroundColor: '#F8F8F8'
  },
  heading: {
    color: '#5D5E6D',
    fontWeight: 500
  },
  row: {
    backgroundColor: '#FFFFFF',
    ':hover': {
      backgroundColor: '#F8F8F8'
    }
  },
  cell: {
    padding: 0
  },
  content: {
    boxSizing: 'border-box',
    minHeight: '40px',
    padding: '12px 20px',
    lineHeight: '16px'
  }
});
