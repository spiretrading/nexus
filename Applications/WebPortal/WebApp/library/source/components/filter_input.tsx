import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The value of the filter. */
  value: string;

  /** The placeholder text. Defaults to 'Filter'. */
  placeholder?: string;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: string) => void;
}

/** A text input with a magnifying glass icon for filtering. */
export function FilterInput(props: Properties): JSX.Element {
  const onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    props.onChange?.(event.target.value);
  };
  return (
    <input type='text' placeholder={props.placeholder ?? 'Filter'}
      value={props.value}
      onChange={onChange}
      className={css(STYLES.input)}/>);
}

const STYLES = StyleSheet.create({
  input: {
    boxSizing: 'border-box',
    fontFamily: 'Roboto',
    fontSize: '14px',
    color: '#000000',
    backgroundColor: '#FFFFFF',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    width: '100%',
    height: '34px',
    padding: '0 34px 0 10px',
    outline: 'none',
    backgroundImage: 'url("resources/magnifying-glass.svg")',
    backgroundPosition: 'right 10px center',
    backgroundSize: '14px 14px',
    backgroundRepeat: 'no-repeat',
    '::placeholder': {
      color: '#8C8C8C'
    }
  }
});
