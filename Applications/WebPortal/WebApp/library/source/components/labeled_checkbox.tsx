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
export class LabeledCheckbox extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.checkboxId = props.id ?? `labeled-checkbox-${nextId++}`;
  }

  public render(): JSX.Element {
    return (
      <div className={css(STYLES.container)}>
        <input type='checkbox' id={this.checkboxId}
          checked={this.props.isChecked}
          onChange={this.onChange}
          className={css(STYLES.input)}/>
        <label htmlFor={this.checkboxId} className={css(STYLES.label)}>
          <Checkbox
            checked={this.props.isChecked}/>
          <span className={css(STYLES.labelText)}>
            {this.props.label}
          </span>
        </label>
      </div>);
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.onChange?.(event.target.checked);
  };

  private checkboxId: string;
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
