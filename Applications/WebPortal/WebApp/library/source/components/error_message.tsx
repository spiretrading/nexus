import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Button } from './button';

interface Properties {

  /** The displayed message. */
  message: string;

  /** Called when the user requests to retry. */
  onRetry?: () => void;
}

/** Displays an error message with a retry button. */
export function ErrorMessage(props: Properties): JSX.Element {
  return (
    <div className={css(STYLES.container)}>
      <div className={css(STYLES.content)}>
        <div className={css(STYLES.filler)}/>
        <img src='resources/components/error.svg'
          className={css(STYLES.errorIcon)}/>
        <div className={css(STYLES.spacer)}/>
        <span className={css(STYLES.message)}>{props.message}</span>
        <div className={css(STYLES.inlineRetry)}>
          <div className={css(STYLES.spacer)}/>
          <Button label='Retry' onClick={props.onRetry}
            style={{width: '246px'}}/>
        </div>
        <div className={css(STYLES.filler)}/>
      </div>
      <div className={css(STYLES.actions)}>
        <Button label='Retry' onClick={props.onRetry}
          style={{width: '100%'}}/>
      </div>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size',
    fontFamily: 'Roboto',
    backgroundColor: '#FFFFFF',
    display: 'flex',
    flexDirection: 'column',
    height: '224px'
  },
  content: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    flexGrow: 1
  },
  filler: {
    flexGrow: 1,
    flexShrink: 1,
    flexBasis: 0
  },
  errorIcon: {
    width: '44px',
    height: '44px',
    flexShrink: 0
  },
  spacer: {
    height: '18px',
    flexShrink: 0
  },
  message: {
    textAlign: 'center',
    color: '#333333',
    fontSize: '0.875rem'
  },
  inlineRetry: {
    display: 'none',
    '@container (min-width: 732px)': {
      display: 'contents'
    }
  },
  actions: {
    position: 'fixed',
    bottom: 0,
    left: 0,
    right: 0,
    display: 'flex',
    justifyContent: 'center',
    backgroundColor: '#FFFFFF',
    padding: '18px 18px 30px',
    boxShadow: '0 0 6px rgb(0 0 0 / 25%)',
    '@container (min-width: 732px)': {
      display: 'none'
    }
  }
});
