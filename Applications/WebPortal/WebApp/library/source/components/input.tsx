import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends React.InputHTMLAttributes<HTMLInputElement> {}

/** A styled input component. */
export const Input = React.forwardRef<HTMLInputElement, Properties>(
    ({readOnly, disabled, className, ...rest}, ref) => {
  return (
    <input
      {...rest}
      ref={ref}
      readOnly={readOnly}
      disabled={disabled}
      className={[css(STYLES.input,
        disabled && STYLES.inputDisabled,
        readOnly && STYLES.inputReadonly), className].join(' ')}/>);
});

const STYLES = StyleSheet.create({
  input: {
    backgroundColor: '#FFFFFF',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    fontSize: '0.875rem',
    fontFamily: "'Roboto', sans-serif",
    padding: '3px 9px',
    color: '#000000',
    outline: 'none',
    height: '34px',
    boxSizing: 'border-box',
    ':hover': {
      borderColor: '#684BC7'
    },
    ':focus': {
      borderColor: '#684BC7'
    },
    ':invalid': {
      borderColor: '#E63F44'
    },
    ':user-invalid': {
      borderColor: '#E63F44'
    },
    '::placeholder': {
      opacity: 1,
      color: '#8C8C8C'
    }
  },
  inputDisabled: {
    opacity: 0.4,
    cursor: 'not-allowed',
    pointerEvents: 'none',
    ':hover': {
      borderColor: '#C8C8C8'
    },
    ':focus': {
      borderColor: '#C8C8C8'
    }
  },
  inputReadonly: {
    borderColor: 'transparent',
    ':hover': {
      borderColor: 'transparent'
    },
    ':focus': {
      borderColor: 'transparent'
    }
  }
});
