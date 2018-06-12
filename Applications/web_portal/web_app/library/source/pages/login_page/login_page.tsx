import * as Beam from 'beam';
import * as React from 'react';
import {LoginPageModel} from '.';

export interface Properties {

  /** The model used to login. */
  model: LoginPageModel;

  /** The event handler called upon a successful login. */
  onLogin?: (account: Beam.DirectoryEntry) => void;
}

export interface State {
  username: string;
  password: string;
};

/** Displays the login page. */
export class LoginPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      username: "",
      password: ""
    };
  }

  public render(): JSX.Element {
    return (
        <div>
          <input type='text' value={this.state.username}
                 placeholder='Username'
                 onChange={(event: any) => {
                   this.setState({username: event.target.value})
                 }}/>
          <input type='text' value={this.state.password}
                 placeholder='Password'
                 onChange={(event: any) => {
                   this.setState({password: event.target.value})
                 }}/>
          <button onClick={() => {
            const login = this.props.model.login.bind(this.props.model)
            login(this.state.username, this.state.password).
              then((account: Beam.DirectoryEntry) => {
                if(this.props.onLogin) {
                  this.props.onLogin(account)
                }
              });
          }}>
            Sign In
          </button>
        </div>
      );
  }
}
