import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** Determines how a form gets submitted depending on the account's roles. */
  roles: Nexus.AccountRoles;

  /** Indicates the form should be submitted. */
  onClick?: () => void;
}

/** Displays the components needed to submit an account related form. */
export class SubmissionBox extends React.Component<Properties> {
  public render(): JSX.Element {
    return null;
  }
}
