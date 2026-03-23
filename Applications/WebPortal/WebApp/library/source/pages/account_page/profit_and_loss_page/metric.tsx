import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Skeleton } from '../../..';

interface Properties {

  /** A unique identifier used to associate the label with the value. */
  id: string;

  /** The label for the metric. */
  label: string;

  /** The value of the metric. */
  value: string;

  /** The unit of the metric. */
  unit?: string;

  /** Whether the metric is loading. */
  loading?: boolean;
}

/** Displays a labeled metric value with an optional unit. */
export function Metric(props: Properties) {
  const valueId = `${props.id}-value`;
  const valueDisplay = (() => {
    if(props.loading) {
      return (
        <div className={css(STYLES.skeletonWrapper)}>
          <Skeleton style={SKELETON_FILL}/>
        </div>);
    }
    return (
      <div className={css(STYLES.valueGroup)}>
        <output id={valueId} className={css(STYLES.value)}>
          {props.value}
        </output>
        {props.unit &&
          <span className={css(STYLES.unit)}>{props.unit}</span>}
      </div>);
  })();
  return (
    <div className={css(STYLES.container)}>
      <label htmlFor={valueId} className={css(STYLES.label)}>
        {props.label}
      </label>
      {valueDisplay}
    </div>);
}

const SKELETON_FILL: React.CSSProperties = {
  width: '100%',
  height: '100%'
};

const STYLES = StyleSheet.create({
  container: {
    display: 'flex',
    flexDirection: 'column',
    minWidth: '60px',
    '@container (min-width: 768px)': {
      minWidth: '80px',
      gap: '4px'
    }
  },
  label: {
    fontSize: '0.75rem',
    fontWeight: 500,
    color: '#7D7E90',
    textTransform: 'uppercase',
    '@container (min-width: 768px)': {
      fontSize: '0.875rem'
    }
  },
  valueGroup: {
    display: 'flex',
    alignItems: 'baseline',
    gap: '4px'
  },
  value: {
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#5D5E6D',
    '@container (min-width: 768px)': {
      fontSize: '1.25rem'
    }
  },
  unit: {
    fontSize: '0.75rem',
    fontWeight: 500,
    color: '#7D7E90',
    '@container (min-width: 768px)': {
      fontSize: '0.875rem'
    }
  },
  skeletonWrapper: {
    width: '60px',
    height: '16px',
    '@container (min-width: 768px)': {
      width: '80px',
      height: '24px'
    }
  }
});
