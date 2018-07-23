import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** Determines what the button's label is. */
  roles: Nexus.AccountRoles;

  /** Determines if the button is disabled.*/
  isDisabled?: boolean;

  /** Indicates the button was clicked. */
  onClick?: () => void;
}

/** Displays the submission button. */
export class SubmitButton extends React.Component<Properties> {
  public defaultProps = {
    isDisabled: false,
    onClick: () => {}
  };
  
  constructor(props: Properties) {
    super(props);
    this.buttonText = (() => {
      if(this.props.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return 'Save Changes';
      }
      return 'Submit Request';
    })();
  }

  public componentWillReceiveProps(newProps: Properties) {
    this.buttonText = (() => {
      if(newProps.roles.isSet(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
        return 'Save Changes';
      }
      return 'Submit Request';
    })();
    this.forceUpdate();
  }

  public render(): JSX.Element {
    return (
      <button disabled={this.props.isDisabled}
        className={css(SubmitButton.STYLE.button)}
        onClick={this.props.onClick}>
        {this.buttonText}
      </button>);
  }
  private static STYLE = StyleSheet.create({
    button: {
      width: '246px',
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
  private buttonText: string;
}
