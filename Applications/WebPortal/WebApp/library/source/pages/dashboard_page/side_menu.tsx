import { Padding, VBoxLayout } from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { SideMenuButton } from './side_menu_button';

interface Properties {

  /** Determines what actions are available based on the account's roles. */
  roles: Nexus.AccountRoles;

  /** Indicates a menu item was clicked. */
  onClick?: (item: SideMenu.Item) => void;
}

/** Display's the dashboard's side menu. */
export class SideMenu extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <VBoxLayout style={SideMenu.STYLE}>
        <Padding size='15px'/>
        <SideMenuButton icon='resources/dashboard/menu-icons/my-profile.svg'
          label='My Profile' href='/account/profile'
          onClick={() => this.props.onClick(SideMenu.Item.PROFILE)}/>
        <SideMenuButton icon='resources/dashboard/menu-icons/accounts.svg'
          label='Accounts' href='/account_directory'
          onClick={() => this.props.onClick(SideMenu.Item.ACCOUNTS)}/>
        <SideMenuButton icon='resources/dashboard/menu-icons/portfolio.svg'
          label='Portfolio' href='/portfolio'
          onClick={() => this.props.onClick(SideMenu.Item.PORTFOLIO)}/>
        <SideMenuButton
          icon='resources/dashboard/menu-icons/request-history.svg'
          label='Request History' href='/request_history'
          onClick={() => this.props.onClick(SideMenu.Item.REQUEST_HISTORY)}/>
        <SideMenuButton icon='resources/dashboard/menu-icons/sign-out.svg'
          label='Sign Out'
          onClick={() => this.props.onClick(SideMenu.Item.SIGN_OUT)}/>
      </VBoxLayout>);
  }

  private static defaultProps = {
    onClick: () => {}
  };
  private static readonly STYLE = {
    width: '200px',
    padding: 0,
    height: '100%',
    minHeight: '568px',
    backgroundColor: '#4B23A0'
  } as React.CSSProperties
}

export namespace SideMenu {

  /** The list of side menu items that can be selected. */
  export enum Item {

    /** The profile item. */
    PROFILE,

    /** The accounts item. */
    ACCOUNTS,

    /** The portfolio item. */
    PORTFOLIO,

    /** The request history item. */
    REQUEST_HISTORY,

    /** The sign out item. */
    SIGN_OUT
  }
}
