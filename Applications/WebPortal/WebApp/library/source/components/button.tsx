import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The text to display on the button. */
  label: string;

  /** The class name of the input box. */
  className?: string;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Additional CSS styles. */
  style?: React.CSSProperties;

  /** Called when the value changes. */
  onClick?: () => void;
}

/** A custom button component. */
export function Button(props: Properties): JSX.Element {
  const style = (() => {
    if(props.readonly) {
      return STYLE.readonly;
    } else {
      return STYLE.default;
    }
  })();
  return (
    <button style={{...style, ...props.style}}
        disabled={props.readonly}
        className={[css(EXTRA_STYLE.button), props.className].join(' ')}
        onClick={props.onClick}>
      {props.label}
    </button>);
}

const STYLE = {
  default: {
    boxSizing: 'border-box',
    height: '34px',
    width: '246px',
    backgroundColor: '#684BC7',
    color: '#FFFFFF',
    border: '0px solid #684BC7',
    borderRadius: '1px',
    font: '400 16px Roboto',
    outline: 'none',
    MozAppearance: 'none',
    cursor: 'pointer'
  } as React.CSSProperties,
  readonly: {
    boxSizing: 'border-box',
    height: '34px',
    width: '246px',
    backgroundColor: '#F8F8F8',
    color: '#8C8C8C',
    border: '0px solid #F8F8F8',
    borderRadius: '1px',
    font: '400 16px Roboto',
    outline: 'none',
    MozAppearance: 'none',
    cursor: 'default'
  } as React.CSSProperties
};

const EXTRA_STYLE = StyleSheet.create({
  button: {
    ':active': {
      backgroundColor: '#4B23A0'
    },
    ':focus': {
      border: 0,
      outline: 'none',
      borderColor: '#4B23A0',
      backgroundColor: '#4B23A0',
      boxShadow: 'none',
      webkitBoxShadow: 'none',
      outlineColor: 'transparent',
      outlineStyle: 'none',
      MozAppearance: 'none'
    },
    ':hover': {
      backgroundColor: '#4B23A0'
    },
    '::-moz-focus-inner': {
      border: 0,
      outline: 0
    },
    ':-moz-focusring': {
      outline: 0
    }
  }
});
