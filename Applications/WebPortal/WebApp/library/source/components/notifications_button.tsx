import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** Whether the current page is the Notifications Page. */
  isCurrent?: boolean;

  /** Whether there are unread notifications. */
  hasUnread?: boolean;

  /** Whether the popover is open. */
  isOpen?: boolean;

  /** Called when the button is clicked. */
  onClick?: () => void;
}

/** A bell icon button that indicates unread notifications. */
export function NotificationsButton(props: Properties): JSX.Element {
  const hasUnread = props.hasUnread ?? false;
  const fillColor = (() => {
    if(props.isCurrent) {
      return '#4B23A0';
    }
    return undefined;
  })();
  const bellStyle = fillColor ? {backgroundColor: fillColor} : undefined;
  return (
    <button aria-label='Notifications'
        className={css(STYLES.button, props.isOpen && STYLES.buttonOpen)}
        onClick={props.onClick}>
      <div aria-hidden='true' className={css(STYLES.bell)} style={bellStyle}/>
      {hasUnread && <span aria-hidden='true' className={css(STYLES.dot)}/>}
    </button>);
}

const STYLES = StyleSheet.create({
  button: {
    position: 'relative',
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    width: '30px',
    height: '30px',
    borderRadius: '30px',
    border: '1px solid transparent',
    backgroundColor: 'transparent',
    outline: 'none',
    padding: 0,
    cursor: 'pointer',
    boxSizing: 'border-box',
    ':hover': {
      backgroundColor: '#E6E6E6'
    },
    ':focus-visible': {
      borderColor: '#684BC7'
    },
    ':hover > div[aria-hidden]': {
      backgroundColor: '#7D7E90'
    },
    ':focus > div[aria-hidden]': {
      backgroundColor: '#7D7E90'
    }
  },
  buttonOpen: {
    backgroundColor: '#E6E6E6'
  },
  bell: {
    width: '15px',
    height: '20px',
    backgroundColor: '#DBDBDB',
    WebkitMaskImage: 'url("resources/components/bell.svg")',
    maskImage: 'url("resources/components/bell.svg")',
    WebkitMaskSize: 'contain',
    maskSize: 'contain',
    WebkitMaskRepeat: 'no-repeat',
    maskRepeat: 'no-repeat'
  },
  dot: {
    position: 'absolute',
    top: '3px',
    right: '4px',
    width: '10px',
    height: '10px',
    backgroundColor: '#007AE6',
    borderRadius: '10px',
    border: '2px solid #FFFFFF',
    boxSizing: 'border-box'
  }
});
