import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The color of the spinner segments. */
  color?: string;

  /** The size of the spinner in pixels. */
  size?: number;
}

/** Displays a segmented loading spinner. */
export function SegmentedSpinner(props: Properties) {
  const color = props.color ?? '#000000';
  const size = props.size ?? 16;
  const segmentWidth = size * 0.25;
  const segmentHeight = size * 0.625;
  const offset = -(size / 2 - segmentHeight / 2);
  const segments: React.ReactElement[] = [];
  for(let i = 0; i < SEGMENT_COUNT; ++i) {
    segments.push(
      <div key={i} className={css(STYLES.segment)} style={{
        width: `${segmentWidth}px`,
        height: `${segmentHeight}px`,
        left: `${(size - segmentWidth) / 2}px`,
        top: `${(size - segmentHeight) / 2}px`,
        backgroundColor: color,
        transform: `rotate(${i * 45}deg) translateY(${offset}px)`,
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
    position: 'relative'
  },
  segment: {
    position: 'absolute',
    transformOrigin: 'center center',
    borderRadius: '1px',
    animationName: FADE,
    animationDuration: '480ms',
    animationTimingFunction: 'linear',
    animationIterationCount: 'infinite'
  }
});
