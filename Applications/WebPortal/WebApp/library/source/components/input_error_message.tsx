import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ValidationError } from '../models';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLSpanElement>, 'children'> {

  /** The validation error to display. Defaults to NONE. */
  error?: ValidationError;

  /** The input label used for REQUIRED, DUPLICATE, and PAST_DATE errors. */
  label?: string;

  /** The expected value type used for FORMAT errors, such as "email". */
  value?: string;

  /** The start value's name used for OUT_OF_RANGE errors. */
  start?: string;

  /** The end value's name used for OUT_OF_RANGE errors. */
  end?: string;
}

/** Displays a validation error message for an input. */
export function InputErrorMessage(props: Properties): JSX.Element {
  const {error = ValidationError.NONE, label = '', value = '', start = '',
    end = '', className, ...attributes} = props;
  const message = (() => {
    switch(error) {
      case ValidationError.REQUIRED:
        return `${label} cannot be empty`;
      case ValidationError.DUPLICATE:
        return `${label} is already in use`;
      case ValidationError.FORMAT:
        return `A valid ${value} is required`;
      case ValidationError.PAST_DATE:
        return `${label} must be in the future`;
      case ValidationError.OUT_OF_RANGE:
        return `${end} must be greater than ${start}`;
      default:
        return '';
    }
  })();
  if(!message) {
    return null;
  }
  return <span {...attributes}
      className={[css(STYLES.message), className].join(' ')}>
    {message}
  </span>;
}

const STYLES = StyleSheet.create({
  message: {
    color: '#E63F44',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontWeight: 400,
    fontSize: '0.875rem'
  }
});
