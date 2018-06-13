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
  error: Error;
};

/** Displays the login page. */
export class LoginPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      username: "",
      password: "",
      error: null
    };
    this.onLogin = this.onLogin.bind(this);
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
          <button onClick={this.onLogin}>
            Sign In
          </button>
        </div>
      );
  }

  private async onLogin() {
    try {
      const account = await this.props.model.login.bind(this.props.model)
                              (this.state.username, this.state.password);
    } catch(error) {
      console.log('error: ', error)
      this.setState({error: error});
    }
  }
}
