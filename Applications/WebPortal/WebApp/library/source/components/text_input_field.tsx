import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../display_size';

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: string;

  /** Text to show if the value is empty. */
  placeholder?: string;

  /** Indicates if there is an error with the value. */
  isError?: boolean;

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the input box. */
  className?: string;

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onInput?: (value: string) => void;
}

/** Displays a single text input field. */
export class TextInputField extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: '',
    placeholder: '',
    isError: false,
    onInput: (_: string) => {}
  };

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TextInputField.STYLE.boxSmall;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return TextInputField.STYLE.boxMedium;
      } else {
        return TextInputField.STYLE.boxLarge;
      }
    })();
    const errorStyle = (() => {
      if(this.props.isError) {
        return TextInputField.EXTRA_STYLE.errorBox;
      } else {
        return null;
      }
    })();
    return (
      <input value={this.props.value}
        disabled={this.props.readonly}
        style={{...boxStyle, ...this.props.style}}
        placeholder={this.props.placeholder}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
          this.props.onInput(event.target.value);}}
        className={css(TextInputField.EXTRA_STYLE.effects, errorStyle) + ' ' +
          this.props.className}/>);
  }

  private static readonly STYLE = {
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
      color: '#333333',
      flexGrow: 1,
      minWidth: '284px',
      width: '100%',
      paddingLeft: '10px',
      backgroundColor: '#FFFFFF'
    },
    boxMedium: {
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
      color: '#333333',
      minWidth: '284px',
      paddingLeft: '10px',
      backgroundColor: '#FFFFFF'
    },
    boxLarge: {
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
      color: '#333333',
      minWidth: '350px',
      paddingLeft: '10px',
      backgroundColor: '#FFFFFF'
    }
  };

  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
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
    },
    inputStyle: {
      flexGrow: 1
    }
  });
}
