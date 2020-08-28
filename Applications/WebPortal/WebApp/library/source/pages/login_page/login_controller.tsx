import * as Beam from 'beam';
import * as React from 'react';
import { LoginModel } from './login_model';
import { LoginPage } from './login_page';

interface Properties {

  /** The model used to login. */
  model: LoginModel;

  /** Indicates a successful login. */
  onLogin?: (account: Beam.DirectoryEntry) => void;
}

interface State {
  status: LoginPage.Status;
  errorMessage: string;
}

/** Implements the controller for the login page. */
export class LoginController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      status: LoginPage.Status.NONE,
      errorMessage: null
    };
  }

  public render(): JSX.Element {
    return <LoginPage status={this.state.status}
      errorMessage={this.state.errorMessage} onSubmit={this.onSubmit}/>;
  }

  private onSubmit = async (username: string, password: string) => {
    this.setState({
      status: LoginPage.Status.LOADING,
      errorMessage: null
    });
    try {
      const account = await this.props.model.login(username, password);
      this.setState({
        status: LoginPage.Status.NONE,
        errorMessage: null
      });
      this.props.onLogin(account);
    } catch(error) {
      this.setState({
        status: LoginPage.Status.NONE,
        errorMessage: error.toString()
      });
    }
  }
}
