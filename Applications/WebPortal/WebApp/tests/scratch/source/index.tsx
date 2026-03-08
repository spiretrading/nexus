import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import * as ReactDOM from 'react-dom';

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size',
    width: '100%',
    border: '1px solid #999',
    resize: 'horizontal',
    overflow: 'auto',
    padding: '20px'
  },
  narrow: {
    backgroundColor: 'red',
    padding: '20px',
    color: '#FFFFFF',
    '@container (min-width: 768px)': {
      display: 'none'
    }
  },
  wide: {
    display: 'none',
    backgroundColor: 'blue',
    padding: '20px',
    color: '#FFFFFF',
    '@container (min-width: 768px)': {
      display: 'block'
    }
  },
  mediumOnly: {
    backgroundColor: 'green',
    padding: '10px',
    color: '#FFFFFF',
    '@container (min-width: 1036px)': {
      display: 'none'
    }
  },
  largeOnly: {
    display: 'none',
    backgroundColor: 'purple',
    padding: '10px',
    color: '#FFFFFF',
    '@container (min-width: 1036px)': {
      display: 'block'
    }
  }
});

ReactDOM.render(
  <div className={css(STYLES.container)}>
    <div className={css(STYLES.narrow)}>NARROW (red) - hidden at 768px</div>
    <div className={css(STYLES.wide)}>WIDE (blue) - shown at 768px</div>
    <div className={css(STYLES.mediumOnly)}>MEDIUM (green) - hidden at 1036px</div>
    <div className={css(STYLES.largeOnly)}>LARGE (purple) - shown at 1036px</div>
  </div>,
  document.getElementById('main'));
