import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {Transition} from 'react-transition-group';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..';
import {DashboardModel} from '.';
import {NotificationButton} from './notification_button';
import {SideMenu} from './side_menu';

interface Properties {

  /** The model to display. */
  model: DashboardModel;

  /** The action to perform on a sign out. */
  onSignOut?: () => void;
}

interface State {
  isSideMenuOpen: boolean;
  isLoading: boolean;
}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  public static defaultProps = {
    onGoToProfile: () => {},
    onGoToAccounts: () => {},
    onGoToPortfolio: () => {},
    onGoToRequestHistory: () => {},
    onSignOut: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      isSideMenuOpen: false,
      isLoading: false
    };
    this.onToggleMenuIsOpen = this.onToggleMenuIsOpen.bind(this);
  }
  
  public componentWillMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoading: false
      });
    });
  }

  public render(): JSX.Element {
    if(this.state.isLoading) {
      return <div/>;
    }
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='60px'
            className={css(DashboardPage.STYLE.header)}>
          <Transition in={this.state.isSideMenuOpen}
              timeout={DashboardPage.MENU_TRANSITION_LENGTH_MS}
              component={null}>
            {(status: string) => {
              return (
                <HBoxLayout width='200px' height='60px' className={
                    css([DashboardPage.HEADER_STYLE.base,
                    DashboardPage.HEADER_STYLE.menuIsOpen,
                    DashboardPage.FADE_TRANSITION_STYLE.base,
                    DashboardPage.getFadeTransition(status)])}>
                  <Padding size='18px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='23px'/>
                    <BurgerButton width='20px' height='14px' color='#E2E0FF'
                      onClick={this.onToggleMenuIsOpen}/>
                    <Padding size='23px'/>
                  </VBoxLayout>
                  <Padding size='20px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='15px'/>
                    <img src='resources/dashboard/logo/logo-inverted.png'
                      width='78px' height='30px'/>
                    <Padding size='15px'/>
                  </VBoxLayout>
                </HBoxLayout>);
            }}
          </Transition>
          <HBoxLayout width='200px' height='60px' className={
              css([DashboardPage.HEADER_STYLE.base])}>
            <Padding size='18px'/>
            <VBoxLayout height='60px'>
              <Padding size='23px'/>
              <BurgerButton width='20px' height='14px' color='#684BC7'
                onClick={this.onToggleMenuIsOpen}/>
              <Padding size='23px'/>
            </VBoxLayout>
            <Padding size='20px'/>
            <VBoxLayout height='60px'>
              <Padding size='15px'/>
              <img src='resources/dashboard/logo/logo.png' width='78px'
                height='30px'/>
              <Padding size='15px'/>
            </VBoxLayout>
          </HBoxLayout>
          <Padding/>
          <VBoxLayout height='60px' width='45px' className={
              css(DashboardPage.STYLE.notificationButtonWrapper)}>
            <Padding size='20px'/>
            <NotificationButton items={0} isOpen={false}/>
            <Padding size='20px'/>
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
        <div className={css(DashboardPage.STYLE.separator)}/>
        <Transition in={this.state.isSideMenuOpen}
            timeout={DashboardPage.MENU_TRANSITION_LENGTH_MS}
            component={null}>
          {(status: string) => {
            return (
              <HBoxLayout height='100%' className={css([
                  DashboardPage.STYLE.sideBarWrapper,
                  DashboardPage.FADE_TRANSITION_STYLE.base,
                  DashboardPage.getFadeTransition(status)])}>
                <SideMenu roles={this.props.model.roles}
                  onProfileClick={() => this.onSideMenuButtonClick(
                    this.onGoToProfile)}
                  onAccountsClick={() => this.onSideMenuButtonClick(
                    this.onGoToAccounts)}
                  onPortfolioClick={() => this.onSideMenuButtonClick(
                    this.onGoToPortfolio)}
                  onRequestHistoryClick={() => this.onSideMenuButtonClick(
                    this.onGoToRequestHistory)}
                  onSignOutClick={() => this.onSideMenuButtonClick(
                    this.props.onSignOut)}/>
              </HBoxLayout>);
          }}
        </Transition>
        <Transition in={this.state.isSideMenuOpen}
            timeout={DashboardPage.MENU_TRANSITION_LENGTH_MS}
            component={null}>
          {(status: string) => {
            return (
              <div className={css([DashboardPage.STYLE.dropShaddow,
                DashboardPage.FADE_TRANSITION_STYLE.base,
                DashboardPage.getFadeTransition(status)])}/>);
          }}
        </Transition>
      </VBoxLayout>);
  }

  private static getFadeTransition(status: string) {
    switch(status) {
      case 'entering':
        return DashboardPage.FADE_TRANSITION_STYLE.entering;
      case 'entered':
        return DashboardPage.FADE_TRANSITION_STYLE.entered;
      case 'exiting':
        return DashboardPage.FADE_TRANSITION_STYLE.exiting;
      case 'exited':
        return DashboardPage.FADE_TRANSITION_STYLE.exited;
    }
  }

  private onToggleMenuIsOpen() {
    this.setState({
      isSideMenuOpen: !this.state.isSideMenuOpen
    });
  }

  private onSideMenuButtonClick(methodToCall: () => void) {
    methodToCall();
    this.onToggleMenuIsOpen();
  }

  private onGoToProfile(){

  }

  private onGoToAccounts(){

  }

  private onGoToPortfolio(){

  }

  private onGoToRequestHistory(){

  }
  private static MENU_TRANSITION_LENGTH_MS = 200;
  private static STYLE = StyleSheet.create({
    separator: {
      width: '100%',
      height: '1px',
      backgroundColor: '#E6E6E6',
      position: 'absolute' as 'absolute',
      top: 60,
      left: 0,
      zIndex: -1
    },
    dropShaddow: {
      width: '1px',
      height: '100%',
      position: 'absolute' as 'absolute',
      top: 0,
      zIndex: -1,
      left: '198px',
      '-webkit-box-shadow': '0px 0px 6px 0px rgba(0,0,0,0.4)',
      '-moz-box-shadow': '0px 0px 6px 0px rgba(0,0,0,0.4)',
      boxShadow: '0px 0px 6px 0px rgba(0,0,0,0.4)'
    },
    header: {
      minWidth: '320px'
    },
    sideBarWrapper: {
      position: 'absolute' as 'absolute',
      left: 0,
      top: 60,
      backgroundColor: '#684BC7',
      zIndex: 1
    },
    notificationButtonWrapper: {
      position: 'absolute' as 'absolute',
      top: 0,
      left: 'calc(100% - 45px)'
    }
  });
  private static HEADER_STYLE = StyleSheet.create({
    base: {
      position: 'absolute' as 'absolute',
      top: 0,
      left: 0,
      backgroundColor: '#FFFFFF'
    },
    menuIsOpen: {
      backgroundColor: '#4B23A0',
      zIndex: 1
    }
  });
  private static FADE_TRANSITION_STYLE = StyleSheet.create({
    base: {
      opacity:  0,
      transition: `opacity ${DashboardPage.MENU_TRANSITION_LENGTH_MS}ms `
        + `ease-out`
    },
    entering: {opacity:  1},
    entered: {opacity:  1},
    exiting: {opacity:  0},
    exited: {opacity: 0}
  });
}
