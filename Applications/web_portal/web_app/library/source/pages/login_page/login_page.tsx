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
  isLoading: boolean;
  errorMessage: 'Incorrect username or password.'|'Server is unavailable.';
}

/** Displays the Login Page. */
export class LoginPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isLoading: false,
      errorMessage: null
    };
    this.staticLogo = (
      <object data='resources/login_page/logo-static.svg'
        type='image/svg+xml' className={css(LoginPage.STYLE.logo)}/>);
    this.animatedLogo = (
      <object data='resources/login_page/logo-animated.svg'
        type='image/svg+xml' className={css(LoginPage.STYLE.logo)}/>);
    this.onLogin = this.onLogin.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);
  }

  public componentDidMount(): void {
    window.addEventListener('keydown', this.onKeyDown);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('keydown', this.onKeyDown);
  }
  public render(): JSX.Element {
    const Logo = () => {
      if(this.state.isLoading) {
        return this.animatedLogo;
      } else {
        return this.staticLogo;
      }
    };
    return (
      <Center width='100%' height='100%'
          className={css(LoginPage.STYLE.page)}>
        <HBoxLayout width='320px' height='462px'>
          <Padding size='18px'/>
          <VBoxLayout width='100%' height='100%'>
            <Padding size='60px'/>
            <Logo/>
            <Padding size='60px'/>
            <input type='text' placeholder='Username'
              className={css(LoginPage.STYLE.inputBox)}
              onFocus={() => this.usernameInputField.placeholder=''}
              onBlur={() => this.usernameInputField.placeholder='Username'}
              ref={((ref) => this.usernameInputField = ref)}/>
            <Padding size='20px'/>
            <input type='password' placeholder='Password'
              className={css(LoginPage.STYLE.inputBox)}
              onFocus={() => this.passwordInputField.placeholder=''}
              onBlur={() => this.passwordInputField.placeholder='Password'}
              ref={((ref) => this.passwordInputField = ref)}/>
            <Padding size='50px'/>
            <button className={css(LoginPage.STYLE.signInButton)}
                onClick={this.onLogin}>
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
      this.setState({isLoading: true});
      try {
        const account = await this.props.model.login(
          this.usernameInputField.value, this.passwordInputField.value);
        this.setState({
          isLoading: false,
          errorMessage: null
        });
        this.props.onLogin(account);
      } catch(error) {
        const errorMessage = (() => {
          switch(error.message) {
            case 'Incorrect username or password':
              return 'Incorrect username or password.';
            default:
              return 'Server is unavailable.'
          }
        })();
        this.setState({
          isLoading: false,
          errorMessage: errorMessage
        });
      }
    }
  }

  private onKeyDown(event: KeyboardEvent) {
    if(document.activeElement !== ReactDOM.findDOMNode(
        this.usernameInputField)) {
      if(document.activeElement !== ReactDOM.findDOMNode(
        this.passwordInputField)) {
        this.usernameInputField.focus();
      } else if(event.key === "Enter") {
        this.onLogin();
      }
    }
  }
  private static STYLE = StyleSheet.create({
    page: {
      fontFamily: 'Roboto',
      backgroundColor: '#4B23A0'
    },
    logo: {
      width: '131px',
      height: '50.38px',
      display: 'block' as 'block',
      marginLeft: 'auto',
      marginRight: 'auto'
    },
    inputBox: {
      width: '100%',
      height: '34px',
      borderColor: '#FFFFFF',
      borderWidth: '0px 0px 1px 0px',
      backgroundColor: 'inherit',
      color: '#FFFFFF',
      outline: 0,
      textAlign: 'center',
      fontSize: '16px',
      fontWeight: 'lighter' as 'lighter',
      borderRadius: 0,
      '::placeholder': {
        color: '#FFFFFF',
      },
      '::-moz-placeholder': {
        color: '#FFFFFF',
        opacity: 1,
      },
      '::-ms-input-placeholder': {
        color: '#FFFFFF',
        opacity: 1,
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none' 
      },
    },
    signInButton: {
      width: '284px',
      height: '48px',
      color: '#4B23A0',
      fontSize: '20px',
      backgroundColor: '#E2E0FF',
      fontWeight: 500,
      outline: 0,
      border: 'none',
      ':hover': {
        backgroundColor: '#FFFFFF'
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
}
