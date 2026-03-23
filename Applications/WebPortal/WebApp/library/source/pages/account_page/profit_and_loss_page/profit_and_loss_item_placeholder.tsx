import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { DropDownButton, Skeleton } from '../../..';

/** Displays a loading placeholder for a profit and loss item. */
export function ProfitAndLossItemPlaceholder() {
  return (
    <div className={css(STYLES.container)}>
      <DropDownButton isExpanded={false}/>
      <Skeleton style={CODE_SKELETON}/>
      <div className={css(STYLES.filler)}/>
      <Skeleton style={PNL_SKELETON}/>
    </div>);
}

const CODE_SKELETON: React.CSSProperties = {
  width: '40px',
  height: '20px',
  flexShrink: 0
};

const PNL_SKELETON: React.CSSProperties = {
  width: '80px',
  height: '20px',
  flexShrink: 0
};

const STYLES = StyleSheet.create({
  container: {
    display: 'flex',
    alignItems: 'center',
    gap: '18px',
    height: '40px'
  },
  filler: {
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: '70px'
  }
});
