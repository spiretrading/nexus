import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';
import {SideMenuButton} from './side_menu_button';

interface Properties {

  /** Used to determine what actions are available based on the account's
   *  roles.
   */
  roles: Nexus.AccountRoles;

  /** Indicates the Profile menu item was clicked. */
  onProfileClick?: () => void;

  /** Indicates the Accounts menu item was clicked. */
  onAccountsClick?: () => void;

  /** Indicates the Portfolio menu item was clicked. */
  onPortfolioClick?: () => void;

  /** Indicates the Request History menu item was clicked. */
  onRequestHistoryClick?: () => void;

  /** Indicates the Sign Out menu item was clicked. */
  onSignOutClick?: () => void;
}

interface State {}

interface MenuButtonProps {
  iconSrc: string;
  text: string;
  onClick: () => void;
}

/** Display's the dashboard's side menu. */
export class SideMenu extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return (
      <VBoxLayout className={css(SideMenu.STYLE.sideMenu)}>
        <Padding size='15px'/>
        <SideMenuButton icon='resources/dashboard/my-profile.svg'
          label='My Profile' onClick={this.props.onProfileClick}/>
        <SideMenuButton icon='resources/dashboard/accounts.svg'
          label='Accounts' onClick={this.props.onAccountsClick}/>
        <SideMenuButton icon='resources/dashboard/portfolio.svg'
          label='Portfolio' onClick={this.props.onPortfolioClick}/>
        <SideMenuButton icon='resources/dashboard/request-history.svg'
          label='Request History' onClick={this.props.onRequestHistoryClick}/>
        <SideMenuButton icon='resources/dashboard/sign-out.svg'
          label='Sign Out' onClick={this.props.onSignOutClick}/>
      </VBoxLayout>);
  }

  private static defaultProps = {
    onProfileClick: () => {},
    onAccountsClick: () => {},
    onPortfolioClick: () => {},
    onRequestHistoryClick: () => {},
    onSignOutClick: () => {}
  }

  private static STYLE = StyleSheet.create({
    sideMenu: {
      width: '200px',
      padding: 0,
      height: '100%',
      minHeight: '568px',
      backgroundColor: '#4B23A0'
    }
  });
}