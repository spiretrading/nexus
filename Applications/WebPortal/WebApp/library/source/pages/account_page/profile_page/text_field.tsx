import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../../';
import { HBoxLayout } from '../../../layouts';

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
    const textStyle = ( () => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TextField.STYLE.largerText;
      } else {
        return TextField.STYLE.text;
      }
    })();
    const imageStyle = ( () => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TextField.STYLE.largerText;
      } else {
        return TextField.STYLE.text;
      }
    })();
    return (
    <div className={css(TextField.STYLE.box)}>
      <input value={this.props.value}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
        this.props.onInput(event.target.value);
      }}
        className={css(TextField.STYLE.text)}/>
      <img src={'resources/account_page/edit.svg'}
        className={css(TextField.STYLE.image)}/>
    </div>);
  }

  private static STYLE = StyleSheet.create({
    box: {
      tabindex: '1',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #FFFFFF',
      ':focus': {
        border: '1px solid #684BC7'
      },
      ':hover': {
        border: '1px solid #C8C8C8'
      }
    },
    image: {
      visibility: 'visible' as 'visible',
      height: '14px',
      width: '14px',
      paddingRight: '10px',
      ':hover': {
        visibility: 'visible' as 'visible'
      }
    },
    text: {
      font: '400 14px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      paddingLeft: '10px',
      border: '1px solid #FFFFFF'
    },
    largerText: {
      font: '400 16px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      border: '1px solid #FFFFFF'
    }
  });
  private static readonly TEXT_PADDING = '10px';
}
