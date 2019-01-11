import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {
  onClick?: () => void;
  isSubmitEnabled?: boolean;
  label: string;
  displaySize: DisplaySize;
}

export class SubmitButton extends React.Component<Properties> {
  public static readonly defaultProps = {
    disabled: true,
    onClick: () => {}
  };

  public render(): JSX.Element {
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SubmitButton.DYNAMIC_STYLE.buttonSmall;
      } else {
        return SubmitButton.DYNAMIC_STYLE.buttonLarge;
      }
    })();
    return (
      <button className={css(buttonStyle)}
          disabled={!this.props.isSubmitEnabled}
          onClick={this.props.onClick}>
        {this.props.label}
      </button>);
  }
  private static DYNAMIC_STYLE = StyleSheet.create({
    buttonSmall: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C'
      }
    },
    buttonLarge: {
      margin: 0,
      padding: 0,
      boxSizing: 'border-box' as 'border-box',
      width: '200px',
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C'
      }
    }
  });
}
