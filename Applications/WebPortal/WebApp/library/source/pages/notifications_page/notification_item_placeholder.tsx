import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Checkbox, Skeleton } from '../../components';

/** Displays a skeleton loading placeholder for a NotificationItem. */
export function NotificationItemPlaceholder(): JSX.Element {
  return (
    <div className={css(STYLES.container)}>
      <div className={css(STYLES.checkboxContainer)}>
        <Checkbox disabled={true}/>
      </div>
      <div className={css(STYLES.body)}>
        <Skeleton style={{width: '138px', height: '18px', flexShrink: 0}}
            className={css(STYLES.descriptionSkeleton)}/>
        <div className={css(STYLES.spacer)}/>
        <Skeleton style={{width: '60px', height: '18px', flexShrink: 0}}
            className={css(STYLES.dateSkeleton)}/>
      </div>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    display: 'flex',
    alignItems: 'center',
    backgroundColor: '#FFFFFF',
    borderBottom: '1px solid #E6E6E6',
    padding: '18px 18px 17px 24px',
    '@container (min-width: 768px)': {
      padding: '10px 18px 9px'
    }
  },
  checkboxContainer: {
    display: 'none',
    flexShrink: 0,
    alignItems: 'center',
    marginRight: '18px',
    '@container (min-width: 768px)': {
      display: 'flex'
    }
  },
  body: {
    display: 'flex',
    alignItems: 'center',
    flex: 1,
    minWidth: 0
  },
  spacer: {
    flex: 1,
    minWidth: '18px'
  },
  descriptionSkeleton: {
    '@container (min-width: 768px)': {
      width: '304px'
    }
  },
  dateSkeleton: {
    '@container (min-width: 768px)': {
      width: '84px'
    }
  }
});
