import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../../';

interface Properties {

  /** Indicates the input field can not be interacted with. */
  disabled?: boolean;

  /** The value to display in the field. */
  value?: string;

  displaySize: DisplaySize;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onInput?: (value: string) => void;

}

/** Displays a single text input field. */
export class TextField extends React.Component<Properties> {
  public static readonly defaultProps = {
    disabled: false,
    value: '',
    onInput: (_: string) => {}
  }

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return <input value={this.props.value}
      className={css(TextField.STYLE.text)}/>;
  }

  private static STYLE = StyleSheet.create({
    text: {
      height: '34px',
      font: '400 14px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      border: '0px solid #684BC7',
      paddingRight: `${TextField.TEXT_PADDING}`,
      paddingLeft: `${TextField.TEXT_PADDING}`,
      ':focus': {
        border: '1px solid #684BC7'
      },
      ':hover': {
        border: '1px solid #C8C8C8'
      }
    },
    largerText: {
      height: '34px',
      font: '400 16px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      border: '0px solid #684BC7',
      paddingRight: `${TextField.TEXT_PADDING}`,
      paddingLeft: `${TextField.TEXT_PADDING}`,
      ':focus': {
        border: '1px solid #684BC7'
      },
      ':hover': {
        border: '1px solid #C8C8C8'
      }
    }
  });
  private static readonly TEXT_PADDING = '10px';
}
