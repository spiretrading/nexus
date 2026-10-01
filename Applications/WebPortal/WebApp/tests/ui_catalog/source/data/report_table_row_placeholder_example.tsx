import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ReportTableRowPlaceholder } from 'web_portal';

/** Demonstrates loading rows in the generated reports table's layout. */
export function ReportTableRowPlaceholderExample(): JSX.Element {
  return <div className={css(STYLES.container)}>
    <table className={css(STYLES.table)}>
      <colgroup className={css(STYLES.columns)}>
        <col style={{width: '48px'}}/>
        <col style={{width: 'min(25cqw, 180px)'}}/>
        <col/>
        <col style={{width: 'min(25cqw, 132px)'}}/>
      </colgroup>
      <tbody className={css(STYLES.body)}>
        <ReportTableRowPlaceholder/>
        <ReportTableRowPlaceholder/>
        <ReportTableRowPlaceholder/>
      </tbody>
    </table>
  </div>;
}

const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%'},
  table: {
    width: '100%',
    maxWidth: '696px',
    borderSpacing: 0,
    tableLayout: 'fixed',
    '@container (width <= 424px)': {display: 'block'}
  },
  columns: {
    '@container (width <= 424px)': {display: 'none'}
  },
  body: {
    '@container (width <= 424px)': {
      display: 'grid',
      rowGap: '20px'
    }
  }
});
