import * as Nexus from 'nexus';
import * as React from 'react';

/** The properties used to render a SideMenu. */
export interface Properties {

  /** Used to determine what actions are available based on the account's
   *  roles.
   */
  roles: Nexus.AccountRoles;

  /** The action to perform when the Profile menu item is selected. */
  onProfileAction: () => void;

  /** The action to perform when the Accounts menu item is selected. */
  onAccountsAction: () => void;

  /** The action to perform when the Portfolio menu item is selected. */
  onPortfolioAction: () => void;

  /** The action to perform when the Request History menu item is selected. */
  onRequestHistoryAction: () => void;

  /** The action to perform when the Sign Out menu item is selected. */
  onSignOutAction: () => void;
}

export interface State {}

/** Display's the dashboard's side menu. */
export class SideMenu extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return null;
  }
}
