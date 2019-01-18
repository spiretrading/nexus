import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {

  /** Indicates a submission is happening submitted. */
  onClick: () => void;

  /** Indicates is the button can be clicked. */
  isSubmitEnabled: boolean;

  /** The text displayed on the button. */
  label: string;

  /** Determines the layout used to display the component. */
  displaySize: DisplaySize;
}

/** Submit button for the profile page. */
export class SubmitButton extends React.Component<Properties> {
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
      borderRadius: 1,
      cursor: 'pointer'  as 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default'  as 'default'
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
      borderRadius: 1,
      cursor: 'pointer'  as 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default'  as 'default'
      }
    }
  });
}
