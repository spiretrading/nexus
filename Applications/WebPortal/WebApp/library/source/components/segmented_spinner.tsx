import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The color of the spinner segments. Defaults to '#000000'. */
  color?: string;

  /** The size of the spinner in pixels. Defaults to 16. */
  size?: number;
}

/** Displays a segmented loading spinner. */
export function SegmentedSpinner(props: Properties) {
  const color = props.color ?? '#000000';
  const size = props.size ?? 16;
  const segments: React.ReactElement[] = [];
  for(let i = 0; i < SEGMENT_COUNT; ++i) {
    segments.push(
      <div key={i} className={css(STYLES.segment)} style={{
        backgroundColor: color,
        transform: `rotate(${i * 45}deg) translateY(var(--offset))`,
        animationDelay: `${i * 60}ms`
      }}/>);
  }
  return (
    <div className={css(STYLES.container)}
      style={{width: `${size}px`, height: `${size}px`}}>
      {segments}
    </div>);
}

const SEGMENT_COUNT = 8;

const FADE = {
  from: {
    opacity: 0.8
  },
  to: {
    opacity: 0
  }
};

const STYLES = StyleSheet.create({
  container: {
    position: 'relative',
    containerType: 'size'
  },
  segment: {
    position: 'absolute',
    width: '12.5cqw',
    height: '31.25cqh',
    top: 'calc(50cqh - 31.25cqh / 2)',
    left: 'calc(50cqw - 12.5cqw / 2)',
    '--offset': 'calc(-1 * (50cqh - 31.25cqh / 2))',
    opacity: 0.8,
    transformOrigin: 'center center',
    animationName: FADE,
    animationDuration: '480ms',
    animationTimingFunction: 'linear',
    animationIterationCount: 'infinite'
  }
});
