import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const STYLES = StyleSheet.create({
  red: {
    backgroundColor: '#E45532',
    height: '100px'
  },
  green: {
    backgroundColor: '#36B24A',
    height: '150px'
  },
  blue: {
    backgroundColor: '#3366CC',
    height: '80px'
  }
});

ReactDOM.render(
  <WebPortal.PageLayout>
    <div>
      <div className={css(STYLES.red)}/>
      <div className={css(STYLES.green)}/>
      <div className={css(STYLES.blue)}/>
    </div>
  </WebPortal.PageLayout>,
  document.getElementById('main'));
