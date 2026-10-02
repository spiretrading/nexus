import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ActivityTableRow } from 'web_portal';

/** Demonstrates an activity row in its containing table. */
export function ActivityTableRowExample(
    props: React.ComponentProps<typeof ActivityTableRow>): JSX.Element {
  return <div className={css(STYLES.container)}>
    <table className={css(STYLES.table)}>
      <colgroup className={css(STYLES.columns)}>
        <col style={{width: '48px'}}/>
        <col style={{width: 'min(20cqw, 180px)'}}/>
        <col/>
        <col style={{width: '100px'}}/>
        <col style={{width: 'min(20cqw, 132px)'}}/>
      </colgroup>
      <tbody className={css(STYLES.body)}>
        <ActivityTableRow {...props}/>
      </tbody>
    </table>
  </div>;
}

const STYLES = StyleSheet.create({
  container: {containerType: 'inline-size', width: '100%'},
  table: {
    width: '100%',
    maxWidth: '796px',
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
