import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../../';
import {LoginPageModel} from '.';

/** The React properties for the LoginPage. */
export interface Properties {

  /** The model used to login. */
  model: LoginPageModel;

  /** The event handler called upon a successful login. */
  onLogin?: (account: Beam.DirectoryEntry) => void;
}

export interface State {
  errorMessage: string;
}

/** Displays the Login Page. */
export class LoginPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      errorMessage: null
    };
    this.onLogin = this.onLogin.bind(this);
    this.onInputChange = this.onInputChange.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);
  }

  public componentDidMount(): void {
    window.addEventListener('keydown', this.onKeyDown);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('keydown', this.onKeyDown);
  }

  public render(): JSX.Element {
    return (
      <Center width='100%' height='100%'
          className={css(LoginPage.STYLE.page)}>
        <HBoxLayout width='320px' height='462px'>
          <Padding size='18px'/>
          <VBoxLayout width='284px' height='462px'>
            <Padding size='60px'/>
            <HBoxLayout width='100%' height='50px'>
              <Padding/>
              <object id='static-logo'
                data='resources/login_page/logo-static.svg'
                type='image/svg+xml' className={
                css(LoginPage.STYLE.logoVisible)}/>
              <object id='animated-logo'
                data='resources/login_page/logo-animated.svg'
                type='image/svg+xml' className={
                css(LoginPage.STYLE.logoInvisible)}>
              </object>
              <Padding/>
           </HBoxLayout>
            <Padding size='60px'/>
            <input type='text' placeholder='Username' autoComplete='off'
              className={css(LoginPage.STYLE.inputBox)}
              onChange={() => this.onInputChange()}
              onFocus={() => this.usernameInputField.placeholder = ''}
              onBlur={() => this.usernameInputField.placeholder = 'Username'}
              ref={((ref) => this.usernameInputField = ref)}/>
            <Padding size='20px'/>
            <input type='password' placeholder='Password' autoComplete='off'
              className={css(LoginPage.STYLE.inputBox)}
              onChange={() => this.onInputChange()}
              onFocus={() => this.passwordInputField.placeholder = ''}
              onBlur={() => this.passwordInputField.placeholder = 'Password'}
              ref={((ref) => this.passwordInputField = ref)}/>
            <Padding size='50px'/>
            <button className={css(LoginPage.STYLE.signInButton)}
                onClick={this.onLogin}
                ref={((ref) => this.submitButton = ref)}>
              Sign In
            </button>
            <Padding size='30px'/>
            <span className={css(LoginPage.STYLE.errorMessage)}>
              {this.state.errorMessage}
            </span>
            <Padding size='60px'/>
          </VBoxLayout>
          <Padding size='18px'/>
        </HBoxLayout>
      </Center>);
  }

  private async onLogin() {
    if(this.usernameInputField.value.trim() !== '') {
      const staticLogo = document.getElementById('static-logo');
      const animatedLogo = document.getElementById('animated-logo');
      staticLogo.className = css(LoginPage.STYLE.logoInvisible);
      animatedLogo.className = css(LoginPage.STYLE.logoVisible);
      try {
        const account = await this.props.model.login(
          this.usernameInputField.value, this.passwordInputField.value);
        staticLogo.className = css(LoginPage.STYLE.logoVisible);
        animatedLogo.className = css(LoginPage.STYLE.logoInvisible);
        this.setState({
          errorMessage: null
        });
        if(this.props.onLogin) {
          this.props.onLogin(account);
        }
      } catch(error) {
        staticLogo.className = css(LoginPage.STYLE.logoVisible);
        animatedLogo.className = css(LoginPage.STYLE.logoInvisible);
        this.setState({
          errorMessage: error.toString()
        });
      }
    }
  }

  private onInputChange() {
    if(this.state.errorMessage) {
      this.setState({errorMessage: null});
    }
  }
  
  private onKeyDown(event: KeyboardEvent) {
    if(document.activeElement !== ReactDOM.findDOMNode(
        this.usernameInputField)) {
      if(document.activeElement !== ReactDOM.findDOMNode(
        this.passwordInputField)) {
        if(document.activeElement === ReactDOM.findDOMNode(
        this.submitButton)) {
          if(event.key === 'Enter') {
            this.onLogin();
          } else {
            this.usernameInputField.focus();
            event.preventDefault();
          }
        } else {
          this.usernameInputField.focus();
          event.preventDefault();
        }
      } else if(event.key === 'Enter') {
        this.onLogin();
      } else if (event.key === 'Tab') {
        this.submitButton.focus();
        event.preventDefault();
      }
    }
  }
  private static STYLE = StyleSheet.create({
    page: {
      fontFamily: 'Roboto',
      backgroundColor: '#4B23A0'
    },
    logoVisible: {
      width: '130px',
      height: '50px'
    },
    logoInvisible: {
      width: '0px',
      height: '0px'
    },
    inputBox: {
      width: '284px',
      padding: 0,
      height: '34px',
      borderColor: '#FFFFFF',
      backgroundColor: '#4B23A0',
      borderWidth: '0px 0px 1px 0px',
      
      color: '#FFFFFF',
      outline: 0,
      textAlign: 'center',
      fontSize: '16px',
      fontWeight: 'lighter' as 'lighter',
      borderRadius: 0,
      '::placeholder': {
        color: '#FFFFFF'
      },
      '::-moz-placeholder': {
        color: '#FFFFFF',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#FFFFFF',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      }
    },
    signInButton: {
      width: '284px',
      height: '48px',
      color: '#4B23A0',
      fontSize: '20px',
      backgroundColor: '#E2E0FF',
      fontWeight: 500,
      outline: 0,
      borderRadius: '2px',
      border: 'none',
      ':hover': {
        backgroundColor: '#FFFFFF'
      },
      ':focus': {
        backgroundColor: '#FFFFFF'
      },
      ':active': {
        backgroundColor: '#FFFFFF'
      },
      '::-moz-focus-inner': {
        border: 0
      }
    },
    errorMessage: {
      width: '100%',
      textAlign: 'center',
      fontSize: '14px',
      height: '20px',
      fontWeight: 'lighter' as 'lighter',
      color: '#FAEB96'
    }
  });
  private staticLogo: JSX.Element;
  private animatedLogo: JSX.Element;
  private usernameInputField: HTMLInputElement;
  private passwordInputField: HTMLInputElement;
  private submitButton: HTMLButtonElement;
}
