import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../../';
import {LoginPageModel} from '.';

/** The React properties for the LoginPage. */
export interface Properties {

  /** The model used to login. */
  model: LoginPageModel;

  /** The event handler called upon a successful login. */
  onLogin?: (account: Beam.DirectoryEntry) => void;
}

/** The React state for the LoginPage. */
export interface State {
  username: string;
  password: string;
  error: Error;
};

/** Displays the Login Page. */
export class LoginPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      username: 'Username',
      password: 'Password',
      error: null
    };
    this.onLogin = this.onLogin.bind(this);
  }

  public render(): JSX.Element {
    const onFocus = (field: string) => {
      switch(field) {
        case 'username':
          if(this.state.username === 'Username') {
            this.setState({username: '', error: null});
          }
          this.setState({error: null});
          break;
        case 'password':
          if(this.state.password === 'Password') {
            this.setState({password: '', error: null});
          }
          this.setState({error: null});
          break;
      }
    };
    const onBlur = (field: string) => {
      switch(field) {
        case 'username':
          if(this.state.username.trim() === '') {
            this.setState({username: 'Username'});
          }
          break;
        case 'password':
          if(this.state.password.trim() === '') {
            this.setState({password: 'Password'});
          }
          break;
      }
    };
    const errorMessage = ((): string => {
      if(this.state.error) {
        return this.state.error.message;
      }
      return '';
    })();
    return (
      <Center width='100%' height='100%'
              className={css(LoginPage.STYLE.page)}>
        <HBoxLayout width='320px' height='462px'>
          <Padding size='18px'/>
          <VBoxLayout width='100%' height='100%'>
            <Padding size='60px'/>
            <img src='resources/login_page/logo-static.svg'
                 className={css(LoginPage.STYLE.logo)}/>
            <Padding size='60px'/>
            <input type='text' value={this.state.username}
                   className={css(LoginPage.STYLE.inputBox)}
                   onBlur={() => onBlur('username')}
                   onFocus={() => onFocus('username')}
                   onChange={(event: any) => {
                     this.setState({username: event.target.value});
                   }}/>
            <Padding size='20px'/>
            <input type='text' value={this.state.password}
                   onBlur={() => onBlur('password')}
                   onFocus={() => onFocus('password')}
                   className={css(LoginPage.STYLE.inputBox)}
                   onChange={(event: any) => {
                     this.setState({password: event.target.value});
                   }}/>
            <Padding size='50px'/>
            <button className={css(LoginPage.STYLE.signInButton)}
                    onClick={this.onLogin}>
              Sign In
            </button>
            <Padding size='30px'/>
            <span className={css(LoginPage.STYLE.errorMessage)}>
              {errorMessage}
            </span>
            <Padding size='60px'/>
          </VBoxLayout>
          <Padding size='18px'/>
        </HBoxLayout>
      </Center>);
  }

  private async onLogin() {
    try {
      const login = this.props.model.login.bind(this.props.model);
      const account = await login(this.state.username, this.state.password);
    } catch(error) {
      this.setState({error: error});
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
      border: 'none',
      borderBottom: '1px solid #FFFFFF',
      backgroundColor: 'inherit',
      color: '#FFFFFF',
      outline: 0,
      textAlign: 'center',
      fontSize: '16px',
      fontWeight: 'lighter' as 'lighter'
    },
    signInButton: {
      width: '284px',
      height: '48px',
      color: '#4B23A0',
      fontSize: '20px',
      backgroundColor: '#FFFFFF',
      outline: 0,
      ':hover': {
        backgroundColor: '#E2E0FF'
      }
    },
    errorMessage: {
      width: '100%',
      textAlign: 'center',
      fontSize: '14px',
      height: '14px',
      fontWeight: 'lighter' as 'lighter',
      color: '#FAEB96'
    }
  });
}
