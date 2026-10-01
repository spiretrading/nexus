import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Checkbox, Skeleton } from '../../../components';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLTableRowElement>, 'children'> {}

/** Displays a generated report's loading row inside a table.
 * The table's containing element must establish an inline-size container.
 */
export function ReportTableRowPlaceholder(
    {className, ...attributes}: Properties): JSX.Element {
  return <tr {...attributes} aria-hidden
      className={[css(STYLES.row), className].join(' ')}>
    <Selected/>
    <TypeCell/>
    <Cell/>
    <Cell/>
  </tr>;
}

function Selected(): JSX.Element {
  return <td className={css(STYLES.selected)}>
    <Checkbox disabled/>
  </td>;
}

function TypeCell(): JSX.Element {
  return <td className={css(STYLES.cell, STYLES.type)}>
    <Skeleton style={{width: '140px', height: '16px'}}/>
  </td>;
}

function Cell(): JSX.Element {
  return <td className={css(STYLES.cell)}>
    <div className={css(STYLES.content)}>
      <Skeleton className={css(STYLES.label)}
        style={{width: '100px', height: '16px', flexShrink: 0}}/>
      <Skeleton className={css(STYLES.value)} style={{height: '16px'}}/>
    </div>
  </td>;
}

const STYLES = StyleSheet.create({
  row: {
    boxSizing: 'border-box',
    backgroundColor: '#FFFFFF',
    '@container (width <= 424px)': {
      display: 'grid',
      gridTemplateColumns: '48px minmax(0, 1fr)',
      border: '1px solid #E6E6E6',
      borderRadius: '1px'
    },
    '@container (424px < width)': {
      border: 'none',
      borderRadius: 0
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
    '@container (width <= 424px)': {
      gridColumn: 2,
      padding: '12px 20px 12px 10px'
    },
    '@container (424px < width)': {padding: '12px 20px'}
  },
  type: {
    '@container (width <= 424px)': {backgroundColor: '#F8F8F8'},
    '@container (424px < width)': {backgroundColor: 'transparent'}
  },
  content: {
    display: 'flex',
    '@container (width <= 424px)': {gap: '8px'}
  },
  label: {
    '@container (424px < width)': {display: 'none'}
  },
  value: {
    flexShrink: 0,
    '@container (width <= 424px)': {width: '112px'},
    '@container (424px < width)': {width: '60%'}
  }
});
