import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends React.HTMLAttributes<HTMLDivElement> {}

/** A placeholder loading skeleton. */
export function Skeleton({className, ...rest}: Properties): JSX.Element {
  return (
    <div
      {...rest}
      className={[css(STYLES.skeleton), className].join(' ')}/>);
}

const SHIFT = {
  from: {
    backgroundPosition: '200% 0'
  },
  to: {
    backgroundPosition: '-200% 0'
  }
};

const STYLES = StyleSheet.create({
  skeleton: {
    width: '80px',
    height: '34px',
    backgroundImage:
      'linear-gradient(270deg, #FAFAFA, #EAEAEA, #EAEAEA, #FAFAFA)',
    backgroundSize: '400% 100%',
    animationName: SHIFT,
    animationDuration: '8s',
    animationTimingFunction: 'ease-in-out',
    animationIterationCount: 'infinite'
  }
});
