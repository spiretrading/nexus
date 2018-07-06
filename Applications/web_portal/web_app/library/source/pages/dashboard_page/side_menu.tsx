import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';

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
        <MenuButton iconSrc='resources/dashboard/my-profile.svg'
          text='My Profile' onClick={this.props.onProfileClick}/>
        <MenuButton iconSrc='resources/dashboard/accounts.svg'
          text='Accounts' onClick={this.props.onAccountsClick}/>
        <MenuButton iconSrc='resources/dashboard/portfolio.svg'
          text='Portfolio' onClick={this.props.onPortfolioClick}/>
        <MenuButton iconSrc='resources/dashboard/request-history.svg'
          text='Request History' onClick={this.props.onRequestHistoryClick}/>
        <MenuButton iconSrc='resources/dashboard/sign-out.svg'
          text='Sign Out' onClick={this.props.onSignOutClick}/>
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

class MenuButton extends React.Component<MenuButtonProps> {
  public render(): JSX.Element {
    return (
      <button onClick={this.props.onClick} className={
          css(MenuButton.STYLE.button)}>
        <HBoxLayout className={
            css(MenuButton.STYLE.buttonContents)}>
          <Padding size='18px'/>
          <Center width='20px' height='40px'>
            <img className={css(MenuButton.STYLE.img)}
              src={this.props.iconSrc}/>
          </Center>
          <Padding size='20px'/>
          <VBoxLayout height='40px'
              className={css(MenuButton.STYLE.buttonText)}>
            <Padding/>
            {this.props.text}
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
      </button>);
  }
  private static STYLE = StyleSheet.create({
    button: {
      width: '200px',
      height: '40px',
      backgroundColor: '#4B23A0',
      outline: 0,
      border: 'none',
      padding: 0,
      font: '200 14px Roboto',
      '-webkit-tap-highlight-color': 'transparent',
      ':hover': {
        backgroundColor: '#684BC7',
        cursor: 'pointer',
        color: 'blue'
      },
      ':active': {
        font: '400 14px Roboto'
      }
    },
    buttonContents: {
      width: '100%',
      height: '40px',
      backgroundColor: 'inherit'
    },
    buttonText: {
      color: 'white',
      font: 'inherit',
      lineHeight: '20px'
    },
    img: {
      width: '20px',
      height: '20px'
    }
  });
}
