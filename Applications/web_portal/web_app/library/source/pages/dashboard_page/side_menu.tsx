import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../..';

/** The properties used to render a SideMenu. */
interface Properties {

  /** Used to determine what actions are available based on the account's
   *  roles.
   */
  roles: Nexus.AccountRoles;

  /** The action to perform when the Profile menu item is selected. */
  onProfileAction?: () => void;

  /** The action to perform when the Accounts menu item is selected. */
  onAccountsAction?: () => void;

  /** The action to perform when the Portfolio menu item is selected. */
  onPortfolioAction?: () => void;

  /** The action to perform when the Request History menu item is selected. */
  onRequestHistoryAction?: () => void;

  /** The action to perform when the Sign Out menu item is selected. */
  onSignOutAction?: () => void;
}

interface State {}

/** Display's the dashboard's side menu. */
export class SideMenu extends React.Component<Properties, State> {
  public render(): JSX.Element {
    interface MenuButtonProps {
      iconSrc: string;
      text: string;
      onClick: () => void;
    }
    const MenuButton = (props: MenuButtonProps): JSX.Element => {
      return (
        <HBoxLayout onClick={props.onClick} className={
            css(SideMenu.STYLE.button)}>
          <Padding size='18px'/>
          <Center width='16px' height='40px'>
            <img className={css(SideMenu.STYLE.img)} src={props.iconSrc}/>
          </Center>
          <Padding size='14px'/>
          <h1 className={css(SideMenu.STYLE.buttonText)}>
            {props.text}
          </h1>
        </HBoxLayout>);
    };
    return (
      <VBoxLayout className={css(SideMenu.STYLE.base)}>
        <MenuButton iconSrc='resources/dashboard/profile.svg'
          text='My Profile' onClick={
            () => this.safeOnClick(this.props.onProfileAction)}/>
        <MenuButton iconSrc='resources/dashboard/accounts.svg'
          text='Accounts' onClick={
            () => this.safeOnClick(this.props.onAccountsAction)}/>
        <MenuButton iconSrc='resources/dashboard/portfolio.svg'
          text='Portfolio' onClick={
            () => this.safeOnClick(this.props.onPortfolioAction)}/>
        <MenuButton iconSrc='resources/dashboard/request-history.svg'
          text='Request History' onClick={
            () => this.safeOnClick(this.props.onRequestHistoryAction)}/>
        <MenuButton iconSrc='resources/dashboard/sign-out.svg'
          text='Sign Out' onClick={
            () => this.safeOnClick(this.props.onSignOutAction)}/>
      </VBoxLayout>);
  }
  private safeOnClick(onClick?: () => void) {
    if(onClick) {
      onClick();
    }
  }
  private static STYLE = StyleSheet.create({
    base: {
      width: '200px',
      height: '100%',
      minHeight: '568px',
      backgroundColor: '#4B23A0'
    },
    button: {
      width: '100%',
      height: '40px',
      backgroundColor: '#4B23A0',
      ':hover': {
        backgroundColor: '#684BC7'
      }
    },
    buttonText: {
      color: 'white',
      font: '200 16px Roboto'
    },
    img: {
      width: '16px',
      height: '16px'
    }
  });
}
