import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {Transition} from 'react-transition-group';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..';
import {DashboardModel} from '.';
import {NotificationButton} from './notification_button';
import {SideMenu} from './side_menu';

/** The properties used to render the DashboardPage. */
interface Properties {

  /** The model to display. */
  model: DashboardModel;

  /** The action to perform when logging out. */
  onLogout?: () => void;
}

interface State {
  isSideMenuOpen: boolean;
}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isSideMenuOpen: false
    };
    this.toggleSideMenuIsOpen = this.toggleSideMenuIsOpen.bind(this);
  }
  public render(): JSX.Element {
    const getFadeTransition = (status: string) => {
      switch(status) {
        case 'entering':
          return DashboardPage.FADE_TRANSITION_STYLE.entering;
        case 'entered':
          return DashboardPage.FADE_TRANSITION_STYLE.entered;
        case 'exiting':
          return  DashboardPage.FADE_TRANSITION_STYLE.exiting;
        case 'exited':
          return  DashboardPage.FADE_TRANSITION_STYLE.exited;
      }
    };
    interface DashboardBurgerButton {
      isMenuOpen: boolean;
    }
    const DashboardBurgerButton = (props: DashboardBurgerButton):
        JSX.Element => {
      const pngSrc = (() => {
        if(props.isMenuOpen) {
          return 'resources/dashboard/burger-light-purple.png';
        }
        return 'resources/dashboard/burger-purple.png';
      })();
      if(window.navigator.userAgent.indexOf('Edge') > -1) {
        return (
          <button className={css(DashboardPage.STYLE.dashboardBurgerButton)}>
            <img className={css(DashboardPage.STYLE.
              dashboardBurgerButtonIcon)}
              onClick={this.toggleSideMenuIsOpen}
              src={pngSrc}/>
          </button>);
      }
      const color = (() => {
        if(props.isMenuOpen) {
          return '#E2E0FF';
        }
        return '#684BC7';
      })();
      return (
        <BurgerButton width='20px' height='14px' color={color}
          highlightColor={color} onClick={this.toggleSideMenuIsOpen}/>);
    };
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
                    getFadeTransition(status)])}>
                  <Padding size='18px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='23px'/>
                    <DashboardBurgerButton isMenuOpen={true}/>
                    <Padding size='23px'/>
                  </VBoxLayout>
                  <Padding size='20px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='15px'/>
                    <img src='resources/dashboard/logo-inverted.png'
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
              <DashboardBurgerButton isMenuOpen={false}/>
              <Padding size='23px'/>
            </VBoxLayout>
            <Padding size='20px'/>
            <VBoxLayout height='60px'>
              <Padding size='15px'/>
              <img src='resources/dashboard/logo.png' width='78px'
                height='30px'/>
              <Padding size='15px'/>
            </VBoxLayout>
          </HBoxLayout>
          <Padding/>
          <VBoxLayout height='60px' width='45px' className={
              css(DashboardPage.STYLE.notificationButtonWrapper)}>
            <Padding size='20px'/>
            <div style={{height: '20px', width: '15px'}}>
              <NotificationButton items={0} isOpen={false}/>
            </div>
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
              <HBoxLayout height='100%' className={ css([
                  DashboardPage.STYLE.sideBarWrapper,
                  DashboardPage.FADE_TRANSITION_STYLE.base,
                  getFadeTransition(status)])}>
                <SideMenu roles={this.props.model.roles}/>
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
                getFadeTransition(status)])}/>);
          }}
        </Transition>
      </VBoxLayout>);
  }

  private toggleSideMenuIsOpen() {
    this.setState({
      isSideMenuOpen: !this.state.isSideMenuOpen
    });
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
    },
    dashboardBurgerButton: {
      width: '20px',
      height: '14px',
      position: 'relative' as 'relative',
      border: 'none',
      cursor: 'pointer',
      backgroundColor: 'inherit',
      outline: 0
    },
    dashboardBurgerButtonIcon: {
      width: '20px',
      height: '14px',
      position: 'absolute' as 'absolute',
      top: 0,
      left: 0
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
