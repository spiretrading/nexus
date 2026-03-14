import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Input } from './input';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>, 'onChange'> {

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: string) => void;
}

/** A text input with a magnifying glass icon for filtering. */
export function FilterInput({onChange, className, ...rest}:
    Properties): JSX.Element {
  const onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    onChange?.(event.target.value);
  };
  return (
    <Input
      type='text'
      placeholder='Filter'
      {...rest}
      onChange={onInputChange}
      className={[css(STYLES.filterInput), className].join(' ')}/>);
}

const STYLES = StyleSheet.create({
  filterInput: {
    width: '100%',
    padding: '0 34px 0 10px',
    backgroundImage: 'url("resources/magnifying-glass.svg")',
    backgroundPosition: 'right 10px center',
    backgroundSize: '14px 14px',
    backgroundRepeat: 'no-repeat'
  }
});
