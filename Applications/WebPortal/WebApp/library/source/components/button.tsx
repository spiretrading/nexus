import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends React.ButtonHTMLAttributes<HTMLButtonElement> {

  /** The text to display on the button. */
  label: string;

  /** The color theme. Defaults to LIGHT. */
  theme?: Button.Theme;
}

/** A custom button component. */
export function Button(props: Properties): JSX.Element {
  const theme = props.theme ?? Button.Theme.LIGHT;
  return (
    <button style={props.style}
        disabled={props.disabled}
        className={[css(EXTRA_STYLE.button,
          theme === Button.Theme.DARK && EXTRA_STYLE.buttonDark,
          props.disabled && EXTRA_STYLE.buttonDisabled,
          props.disabled && theme === Button.Theme.DARK &&
            EXTRA_STYLE.buttonDisabledDark),
          props.className].join(' ')}
        onClick={props.onClick}>
      {props.label}
    </button>);
}

export namespace Button {

  /** The color theme of the button. */
  export enum Theme {

    /** For use on light backgrounds. Default. */
    LIGHT,

    /** For use on dark backgrounds. */
    DARK
  }
}

const EXTRA_STYLE = StyleSheet.create({
  button: {
    backgroundColor: '#684BC7',
    color: '#FFFFFF',
    border: '1px solid transparent',
    borderRadius: '1px',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    padding: '3px 9px',
    cursor: 'pointer',
    outline: 'none',
    ':active': {
      backgroundColor: '#4B23A0'
    },
    ':focus': {
      backgroundColor: '#4B23A0'
    },
    ':hover': {
      backgroundColor: '#4B23A0'
    }
  },
  buttonDark: {
    backgroundColor: '#E2E0FF',
    color: '#4B23A0',
    ':focus': {
      backgroundColor: '#E2E0FF'
    },
    ':hover': {
      backgroundColor: '#FFFFFF'
    },
    ':active': {
      backgroundColor: '#FFFFFF'
    },
    ':focus-visible': {
      backgroundColor: '#B9B4EC',
      border: '1px solid #FFFFFF'
    }
  },
  buttonDisabled: {
    backgroundColor: '#F8F8F8',
    color: '#8C8C8C',
    cursor: 'default',
    ':hover': {
      backgroundColor: '#F8F8F8'
    },
    ':active': {
      backgroundColor: '#F8F8F8'
    }
  },
  buttonDisabledDark: {
    backgroundColor: '#684BC7',
    ':hover': {
      backgroundColor: '#684BC7'
    },
    ':active': {
      backgroundColor: '#684BC7'
    }
  }
});
