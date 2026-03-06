import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The form name for the segmented control. */
  name: string;

  /** The segment buttons the user can select from. */
  children?: React.ReactNode;
}

/** A group of segment buttons displayed as a single control. */
export function SegmentedControl(props: Properties): JSX.Element {
  const children = React.Children.toArray(props.children);
  return (
    <div className={css(STYLES.container)}>
      {children.map((child, index) => {
        const isLast = index === children.length - 1;
        const cloned = React.isValidElement(child) ?
          React.cloneElement(child as React.ReactElement<any>,
            {name: props.name}) : child;
        return (
          <div key={index} className={css(
            STYLES.segment,
            !isLast && STYLES.segmentDivider
          )}>
            {cloned}
          </div>);
      })}
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    display: 'flex',
    border: '1px solid #E3E3E3',
    borderRadius: '1px'
  },
  segment: {
    display: 'grid',
    flex: '1 1 0',
    position: 'relative' as 'relative',
    minWidth: 0,
    ':focus-within': {
      zIndex: 1
    }
  },
  segmentDivider: {
    borderRight: '1px solid #E3E3E3'
  }
});
