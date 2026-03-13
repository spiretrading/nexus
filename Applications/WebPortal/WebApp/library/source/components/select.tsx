import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.SelectHTMLAttributes<HTMLSelectElement>, 'onChange'> {

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Called when the selected value changes.
   * @param value - The new selected value.
   */
  onChange?: (value: string) => void;
}

/** A select dropdown component. */
export function Select({readonly, onChange, className, children, ...rest}:
    Properties): JSX.Element {
  const onSelectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    onChange?.(event.target.value);
  };
  return (
    <select
      {...rest}
      disabled={readonly}
      className={[css(STYLES.select,
        readonly && STYLES.selectReadonly), className].join(' ')}
      onChange={readonly ? undefined : onSelectChange}>
      {children}
    </select>);
}

const STYLES = StyleSheet.create({
  select: {
    backgroundColor: '#F8F8F8',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    fontSize: '0.875rem',
    fontFamily: "'Roboto', system-ui, sans-serif",
    padding: '3px 28px 3px 9px',
    color: '#333333',
    outline: 'none',
    backgroundImage: 'url("resources/components/arrow-down.svg")',
    backgroundSize: '8px 6px',
    backgroundPosition: 'right 10px top 50%',
    backgroundRepeat: 'no-repeat',
    appearance: 'none',
    WebkitAppearance: 'none',
    height: '34px',
    width: 'fit-content',
    ':hover': {
      borderColor: '#684BC7'
    },
    ':focus': {
      borderColor: '#684BC7'
    }
  },
  selectReadonly: {
    borderColor: 'transparent',
    backgroundColor: '#FFFFFF',
    backgroundImage: 'none',
    ':hover': {
      borderColor: 'transparent'
    },
    ':focus': {
      borderColor: 'transparent'
    }
  }
});
