import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.ButtonHTMLAttributes<HTMLButtonElement>, 'children'> {

  /** The path or URL of the SVG icon to display. */
  icon: string;

  /** A reference to the underlying button element. */
  buttonRef?: React.Ref<HTMLButtonElement>;
}

/** An icon-only button. Supply an accessible name using aria-label. */
export function IconButton(props: Properties): JSX.Element {
  const {icon, buttonRef, className, type = 'button', ...attributes} = props;
  const mask = `url(${JSON.stringify(icon)})`;
  return <button {...attributes} ref={buttonRef} type={type}
      className={[css(STYLES.button), className].join(' ')}>
    <span aria-hidden className={css(STYLES.icon)}
      style={{maskImage: mask, WebkitMaskImage: mask}}/>
  </button>;
}

const STYLES = StyleSheet.create({
  button: {
    boxSizing: 'border-box',
    display: 'inline-flex',
    alignItems: 'center',
    justifyContent: 'center',
    width: '20px',
    height: '20px',
    padding: '2px',
    border: '1px solid transparent',
    backgroundColor: 'transparent',
    color: '#684BC7',
    cursor: 'pointer',
    outline: 'none',
    ':hover': {
      backgroundColor: '#F8F8F8',
      color: '#4B23A0'
    },
    ':focus-visible': {borderColor: '#684BC7'},
    ':disabled': {color: '#DBDBDB', cursor: 'not-allowed'}
  },
  icon: {
    display: 'block',
    width: '100%',
    height: '100%',
    backgroundColor: 'currentColor',
    maskSize: 'contain',
    maskPosition: 'center',
    maskRepeat: 'no-repeat',
    WebkitMaskSize: 'contain',
    WebkitMaskPosition: 'center',
    WebkitMaskRepeat: 'no-repeat'
  }
});
