import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../display_size';

interface Properties {

  /** The value to display in the field. */
  value?: number;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: number) => void;
}

/** Displays a single text input field. */
export class DecimalNumberInputField extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: 0.0,
    placeholder: 0.0,
    isError: false,
    onInput: (_: string) => {}
  };

  public render(): JSX.Element {
    const boxStyle = (() => {
      return DecimalNumberInputField.STYLE.boxSmall;
    })();
    return (
      <input value={this.props.value}
        type={'number'}
        step={'0.01'}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
          this.props.onChange(event.target.valueAsNumber);
        }}
        className={css(boxStyle)}/>);
  }

  private static STYLE = StyleSheet.create({
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 1,
      minWidth: '284px',
      width: '100%',
      paddingLeft: '10px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none',
      ':focus': {
        ouline: 0,
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    },
    errorBox: {
      border: '1px solid #E63F44'
    }
  });
}
