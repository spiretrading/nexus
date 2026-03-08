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
  }
});

ReactDOM.render(
  <div className={css(STYLES.container)}>
    <div className={css(STYLES.narrow)}>NARROW (red)</div>
    <div className={css(STYLES.wide)}>WIDE (blue)</div>
  </div>,
  document.getElementById('main'));
