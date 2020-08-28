import { css, StyleSheet } from 'aphrodite';
import * as Dali from 'dali';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { SubmitButton } from './submit_button';

interface Properties {

  /** Determines the layout used to display the component. */
  displaySize: DisplaySize;

  /** Whether the password button can be clicked. */
  isPasswordSubmitEnabled: boolean;

  /** The status of the password submission. */
  submitPasswordStatus: string;

  /** Whether an error occurred submitting the password. */
  hasPasswordError: boolean;

  /** Indicates the password has been updated. */
  onSubmitPassword: () => void;

  /** The value of the first password input field. */
  password1: string;

  /** The value of the second password input field. */
  password2: string;

  /** Indicates the first password field has been changed. */
  onPassword1Change?: (newValue: string) => void;

  /** Indicates the second password field has been changed. */
  onPassword2Change?: (newValue: string) => void;
}

/** Displays a box that allows the user to submit a new password. */
export class ChangePasswordBox extends React.Component<Properties> {
  public render(): JSX.Element {
    const changePasswordBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePasswordBox.STYLE.passwordBoxSmall;
      } else {
        return ChangePasswordBox.STYLE.passwordBoxLarge;
      }
    })();
    const errorStyle = (() => {
      if(this.props.hasPasswordError && this.props.password1 === '') {
        return ChangePasswordBox.DYNAMIC_STYLE.errorBoxState;
      } else {
        return null;
      }
    })();
    const inputBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePasswordBox.DYNAMIC_STYLE.inputBoxSmall;
      } else {
        return ChangePasswordBox.DYNAMIC_STYLE.inputBoxLarge;
      }
    })();
    const messageInline = (() => {
      if(this.props.displaySize === DisplaySize.LARGE) {
        if(this.props.hasPasswordError) {
          return ChangePasswordBox.STYLE.errorMessage;
        } else {
          return ChangePasswordBox.STYLE.statusMessage;
        }
      } else {
        return ChangePasswordBox.STYLE.hidden;
      }
    })();
    const inlineStatusBox = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.LARGE:
          return (
            <div style={{...messageInline,
                ...ChangePasswordBox.STYLE.fillerMessage}}>
              {this.props.submitPasswordStatus}
            </div>);
        case DisplaySize.MEDIUM:
          return <div style={ChangePasswordBox.STYLE.filler}/>;
        case DisplaySize.SMALL:
          return <div style={ChangePasswordBox.STYLE.filler}/>;
      }
    })();
    const messageUnderneath = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.LARGE:
          return ChangePasswordBox.STYLE.hidden;
        case DisplaySize.MEDIUM:
          if(this.props.hasPasswordError) {
            return ChangePasswordBox.STYLE.errorMessage;
          } else {
            return ChangePasswordBox.STYLE.statusMessage;
          }
        case DisplaySize.SMALL:
          if(this.props.hasPasswordError) {
            return {...ChangePasswordBox.STYLE.errorMessage,
              ...ChangePasswordBox.STYLE.passwordBoxSmall};
          } else {
            return {...ChangePasswordBox.STYLE.statusMessage,
              ...ChangePasswordBox.STYLE.passwordBoxSmall};
          }
      }
    })();
    return (
      <Dali.VBoxLayout>
        <div style={ChangePasswordBox.STYLE.headerStyler}>Change Password</div>
        <Dali.Padding size={ChangePasswordBox.STANDARD_PADDING}/>
        <div style={changePasswordBox}>
          <input type='password' placeholder='New Password'
            value={this.props.password1}
            autoComplete='off'
            className={css(inputBoxStyle, errorStyle)}
            disabled={false}
            onFocus={() => this.passwordInputField.placeholder = ''}
            onBlur={() =>
              this.passwordInputField.placeholder = 'New Password'}
            onChange={(event: React.ChangeEvent<HTMLInputElement>) =>
              this.props.onPassword1Change(event.target.value)}
            ref={(ref) => this.passwordInputField = ref}/>
          <div style={ChangePasswordBox.STYLE.passwordPadding}/>
          <input type='password' placeholder='Confirm New Password'
            value={this.props.password2}
            disabled={this.props.password1 === ''}
            autoComplete='off'
            className={css(inputBoxStyle, errorStyle)}
            onChange={(event: React.ChangeEvent<HTMLInputElement>) =>
              this.props.onPassword2Change(event.target.value)}
            onFocus={() => this.confirmPasswordInputField.placeholder = ''}
            onBlur={() =>
              this.confirmPasswordInputField.placeholder
              = 'Confirm New Password'}
            ref={(ref) => this.confirmPasswordInputField = ref}/>
          {inlineStatusBox}
          <SubmitButton label='Save Password'
            displaySize={this.props.displaySize}
            isSubmitEnabled={this.props.isPasswordSubmitEnabled}
            onClick={this.props.onSubmitPassword}/>
        </div>
        <div style={messageUnderneath}>
          <div style={ChangePasswordBox.STYLE.smallPadding}/>
          {status}
        </div>
      </Dali.VBoxLayout>);
  }
  private static readonly STYLE = {
    hidden: {
      opacity: 0,
      visibility: 'hidden',
      display: 'none'
    } as React.CSSProperties,
    headerStyler: {
      color: '#4B23A0',
      font: '400 14px Roboto',
      width: '100%'
    } as React.CSSProperties,
    errorMessage: {
      color: '#E63F44',
      font: '400 14px Roboto'
    } as React.CSSProperties,
    statusMessage: {
      color: '#36BB55',
      font: '400 14px Roboto'
    } as React.CSSProperties,
    filler: {
      flexGrow: 1,
      height: '30px'
    } as React.CSSProperties,
    fillerMessage: {
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      flexGrow: 1,
      textAlign: 'right',
      marginRight: '30px',
      justifyContent: 'flex-end'
    } as React.CSSProperties,
    smallPadding: {
      width: '100%',
      height: '18px'
    } as React.CSSProperties,
    mediumPadding: {
      width: '100%',
      height: '30px'
    } as React.CSSProperties,
    passwordBoxLarge: {
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      alignItems: 'center'
    } as React.CSSProperties,
    passwordBoxSmall: {
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      justifyContent: 'center'
    } as React.CSSProperties,
    passwordPadding: {
      height: '18px',
      width: '18px'
    } as React.CSSProperties,
    passwordButtonPadding: {
      height: '30px',
      width: '30px'
    } as React.CSSProperties
  };
  private static DYNAMIC_STYLE = StyleSheet.create({
    inputBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      padding: 0,
      height: '34px',
      border: '1px solid #C8C8C8',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      outline: 0,
      borderRadius: 1,
      paddingLeft: '10px',
      textAlign: 'start' as 'start',
      ':active': {
        border: '1px solid #684BC7'
      },
      ':focus':{
        border: '1px solid #684BC7'
      },
      '::placeholder': {
        color: '#8C8C8C'
      },
      '::-moz-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      },
      '::-webkit-autofill': {
        backgroundColor: 'none'
      },
      '::-webkit-credentials-auto-fill-button': {
        visibility: 'hidden' as 'hidden',
        display: 'none !important',
        pointerEvents: 'none',
        height: 0,
        width: 0,
        margin: 0
      }
    },
    inputBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      width: '240px',
      padding: 0,
      height: '34px',
      border: '1px solid #C8C8C8',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      outline: 0,
      paddingLeft: '10px',
      textAlign: 'start' as 'start',
      borderRadius: 1,
      ':active': {
        border: '1px solid #684BC7'
      },
      ':focus':{
        border: '1px solid #684BC7'
      },
      '::placeholder': {
        color: '#8C8C8C'
      },
      '::-moz-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#8C8C8C',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      },
      '::-webkit-autofill': {
        backgroundColor: 'none'
      },
      '::-webkit-credentials-auto-fill-button': {
        visibility: 'hidden' as 'hidden',
        display: 'none !important',
        pointerEvents: 'none',
        height: 0,
        width: 0,
        margin: 0
      }
    },
    errorBoxState: {
      borderColor: '#E63F44'
    }
  });
  private static readonly STANDARD_PADDING = '30px';
  private passwordInputField: HTMLInputElement;
  private confirmPasswordInputField: HTMLInputElement;
}
