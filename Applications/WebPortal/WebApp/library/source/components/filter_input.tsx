import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The value of the filter. */
  value: string;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: string) => void;
}

/** A text input with a magnifying glass icon for filtering. */
export class FilterInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <input type='text' placeholder='Filter'
        value={this.props.value}
        onChange={this.onInputChange}
        className={css(STYLES.input)}/>);
  }

  private onInputChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.onChange?.(event.target.value);
  };
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
    height: '34px',
    padding: '0 34px 0 10px',
    outline: 'none',
    backgroundImage: 'url("resources/magnifying-glass.svg")',
    backgroundPosition: 'right 10px center',
    backgroundSize: '14px 14px',
    backgroundRepeat: 'no-repeat',
    '::placeholder': {
      color: '#8C8C8C'
    },
    '::-ms-clear': {
      display: 'none'
    }
  }
});
