import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { DropDownButton, Skeleton } from '../../..';

/** Displays a loading placeholder for a profit and loss item. */
export function ProfitAndLossItemPlaceholder() {
  return (
    <div className={css(STYLES.container)}>
      <DropDownButton isExpanded={false}/>
      <Skeleton style={SKELETON_STYLE}/>
    </div>);
}

const SKELETON_STYLE: React.CSSProperties = {
  width: '100%',
  height: '16px',
  flex: 1
};

const STYLES = StyleSheet.create({
  container: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px',
    height: '40px',
    pointerEvents: 'none',
    opacity: 0.5
  }
});
