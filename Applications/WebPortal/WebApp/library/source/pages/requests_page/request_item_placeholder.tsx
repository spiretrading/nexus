import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { Skeleton } from '../..';

/** Displays a skeleton loading placeholder for a RequestItem. */
export function RequestItemPlaceholder() {
  return (
    <div className={css(STYLES.container)}>
      <div className={css(STYLES.header)}>
        <Skeleton className={css(STYLES.headerLeft)}/>
        <Skeleton className={css(STYLES.headerRight)}/>
      </div>
      <div className={css(STYLES.metadata)}>
        <Skeleton className={css(STYLES.metadataSkeleton)}/>
      </div>
      <div className={css(STYLES.changes)}>
        <Skeleton className={css(STYLES.changesSkeleton)}/>
      </div>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    fontFamily: 'Roboto',
    border: '1px solid transparent',
    borderBottomColor: '#E6E6E6',
    backgroundColor: '#FFFFFF',
    padding: '11px 17px'
  },
  header: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px'
  },
  headerLeft: {
    width: '160px',
    height: '18px',
    flexShrink: 0
  },
  headerRight: {
    width: '60px',
    height: '18px',
    flexShrink: 0,
    marginLeft: 'auto',
    '@container (min-width: 768px)': {
      width: '116px'
    }
  },
  metadata: {
    marginTop: '4px'
  },
  metadataSkeleton: {
    width: '60px',
    height: '16px',
    '@container (min-width: 768px)': {
      width: '284px'
    }
  },
  changes: {
    marginTop: '10px'
  },
  changesSkeleton: {
    width: '100%',
    height: '24px',
    '@container (min-width: 768px)': {
      width: '284px'
    }
  }
});
