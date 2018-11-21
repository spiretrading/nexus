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
}

export class LoginController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {}
  }

  public render(): JSX.Element {
    return <LoginPage/>;
  }
}
