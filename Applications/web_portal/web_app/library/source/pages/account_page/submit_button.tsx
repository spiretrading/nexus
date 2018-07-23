import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** Determines what the button's label is. */
  roles: Nexus.AccountRoles;

  /** Indicates the button was clicked. */
  onClick?: () => void;
}

/** Displays the submission button. */
export class SubmitButton extends React.Component<Properties> {
  public render(): JSX.Element {
    return null;
  }
}
