import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

/** Displays a skeleton loading placeholder for a RequestItem. */
export function RequestItemPlaceholder() {
  return (
    <div className={css(STYLES.container)}>
      <div className={css(STYLES.header)}>
        <div className={css(STYLES.skeleton, STYLES.headerLeft)}/>
        <div className={css(STYLES.skeleton, STYLES.headerRight)}/>
      </div>
      <div className={css(STYLES.metadata)}>
        <div className={css(STYLES.skeleton, STYLES.metadataSkeleton)}/>
      </div>
      <div className={css(STYLES.changes)}>
        <div className={css(STYLES.skeleton, STYLES.changesSkeleton)}/>
      </div>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    fontFamily: 'Roboto',
    border: '1px solid transparent',
    borderBottomColor: '#E6E6E6',
    backgroundColor: '#FFFFFF',
    padding: '11px 17px',
    containerType: 'inline-size'
  },
  skeleton: {
    background: 'linear-gradient(to right, #EAEAEA, #FAFAFA)'
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
