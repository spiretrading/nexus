import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Checkbox } from '..';

let nextId = 0;

interface Properties {

  /** The unique id for the checkbox. Auto-generated if not provided. */
  id?: string;

  /** The label text. */
  label: string;

  /** Whether the checkbox is checked. */
  isChecked: boolean;

  /** Called when the checked state changes.
   * @param isChecked - The new checked state.
   */
  onChange?: (isChecked: boolean) => void;
}

/** A checkbox with an associated label. */
export function LabeledCheckbox(props: Properties): JSX.Element {
  const checkboxId = React.useRef(props.id ?? `labeled-checkbox-${nextId++}`);
  const onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    props.onChange?.(event.target.checked);
  };
  return (
    <div className={css(STYLES.container)}>
      <input type='checkbox' id={checkboxId.current}
        checked={props.isChecked}
        onChange={onChange}
        className={css(STYLES.input)}/>
      <label htmlFor={checkboxId.current} className={css(STYLES.label)}>
        <Checkbox
          checked={props.isChecked}/>
        <span className={css(STYLES.labelText)}>
          {props.label}
        </span>
      </label>
    </div>);
}

const STYLES = StyleSheet.create({
  container: {
    display: 'inline-flex',
    position: 'relative'
  },
  input: {
    position: 'absolute',
    width: '100%',
    height: '100%',
    appearance: 'none',
    WebkitAppearance: 'none',
    MozAppearance: 'none',
    outline: 'none',
    pointerEvents: 'none',
    border: 'none',
    margin: 0,
    ':focus-visible': {
      outline: 'none',
      border: 'none',
      boxShadow: 'none'
    }
  },
  label: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center',
    cursor: 'pointer'
  },
  labelText: {
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    color: '#333333',
    padding: '0 8px',
    userSelect: 'none'
  }
});
