import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>, 'onClick'> {

  /** Called when the check mark is clicked on.
   * @param isChecked - The updated value.
   */
  onClick?: (isChecked: boolean) => void;
}

/** A checkbox component. */
export function Checkbox(props: Properties): JSX.Element {
  const onChange = () => {
    props.onClick?.(!props.checked);
  };
  return (
    <input type='checkbox'
        checked={props.checked}
        disabled={props.disabled}
        className={[css(STYLES.checkbox), props.className].join(' ')}
        onChange={onChange}/>);
}

const STYLES = StyleSheet.create({
  checkbox: {
    appearance: 'none',
    WebkitAppearance: 'none',
    backgroundImage: 'url("resources/components/none.svg")',
    backgroundRepeat: 'no-repeat',
    backgroundPosition: 'center',
    backgroundSize: 'contain',
    border: 'none',
    display: 'block',
    padding: 0,
    boxSizing: 'border-box',
    cursor: 'pointer',
    margin: 0,
    width: '20px',
    height: '20px',
    ':checked': {
      backgroundImage: 'url("resources/components/check.svg")'
    },
    ':indeterminate': {
      backgroundImage: 'url("resources/components/check-indeterminate.svg")'
    },
    ':disabled': {
      cursor: 'not-allowed',
      opacity: 0.4
    },
    '@media (768px <= width)': {
      backgroundSize: '16px 16px'
    }
  }
});
