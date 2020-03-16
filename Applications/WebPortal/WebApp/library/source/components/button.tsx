import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The text to display on the button. */
  label: string;

  /** The class name of the input box. */
  className?: string;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** Additional CSS styles. */
  style?: any;

  /** Called when the value changes. */
  onClick?: () => void;
}

/** A custom button component. */
export class Button extends React.Component<Properties> {
  public static readonly defaultProps = {
    onClick: () => {}
  }

  public render() {
    const style = (() => {
      if(this.props.readonly) {
        return Button.STYLE.readonly;
      } else {
        return Button.STYLE.default;
      }
    })();
    return (
      <button style={{...style, ...this.props.style}}
          disabled={this.props.readonly}
          className={
            [css(Button.EXTRA_STYLE.button), this.props.className].join(' ')}
          onClick={this.props.onClick}>
        {this.props.label}
      </button>);
  }

  private static readonly STYLE = {
    default: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      border: '0px solid #684BC7',
      borderRadius: '1px',
      font: '400 16px Roboto',
      outline: 'none',
      MozAppearance: 'none' as 'none',
      cursor: 'pointer' as 'pointer'
    },
    readonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#F8F8F8',
      color: '#8C8C8C',
      border: '0px solid #F8F8F8',
      borderRadius: '1px',
      font: '400 16px Roboto',
      outline: 'none',
      MozAppearance: 'none' as 'none',
      cursor: 'default' as 'default'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    button: {
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus': {
        border: 0,
        outline: 'none',
        borderColor: '#4B23A0',
        backgroundColor: '#4B23A0',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none',
        MozAppearance: 'none' as 'none'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      },
      '::-moz-focus-inner': {
        border: 0,
        outline: 0
      },
      ':-moz-focusring': {
        outline: 0
      }
    }
  });
}
