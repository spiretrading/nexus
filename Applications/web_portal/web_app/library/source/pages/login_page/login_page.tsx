import * as Beam from 'beam';
import * as React from 'react';
import {LoginPageModel} from '.';

export interface Properties {

  /** The model used to login. */
  model: LoginPageModel;

  /** The event handler called upon a successful login. */
  onLogin?: (account: Beam.DirectoryEntry) => void;
}

export interface State {}

/** Displays the login page. */
export class LoginPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    return <div>Login Page</div>;
  }
}
