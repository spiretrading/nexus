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
    const buttonColor = (() => {
      if(this.state.isSideMenuOpen) {
        return '#FFFFFF';
      }
      return '#684BC7';
    })();
    const buttonHighlightColor = (() => {
      if(this.state.isSideMenuOpen) {
        return '#FFFFFF';
      }
      return '#684BC7';
    })();
    const logoImageSrc = (() => {
      if(this.state.isSideMenuOpen) {
        return 'resources/dashboard/logo-inverted.png';
      }
      return 'resources/dashboard/logo.png';
    })();
    const sideMenuClassName = (status: string): string => {
      switch(status) {
        case 'entering':
          return css([DashboardPage.SIDE_MENU_TRANSITION_STYLE.base,
            DashboardPage.SIDE_MENU_TRANSITION_STYLE.entering]);
        case 'entered':
          return css([DashboardPage.SIDE_MENU_TRANSITION_STYLE.base,
            DashboardPage.SIDE_MENU_TRANSITION_STYLE.entered]);
        case 'exiting':
          return css([DashboardPage.SIDE_MENU_TRANSITION_STYLE.base,
            DashboardPage.SIDE_MENU_TRANSITION_STYLE.exiting]);
      }
    };
    const headerStyle = (status: string) => {
      switch(status) {
        case 'entering':
          return DashboardPage.HEADER_TRANSITION_STYLE.entering;
        case 'entered':
          return DashboardPage.HEADER_TRANSITION_STYLE.entered;
        case 'exiting':
          return  DashboardPage.HEADER_TRANSITION_STYLE.exiting;
        case 'exited':
          return  DashboardPage.HEADER_TRANSITION_STYLE.exited;
      }
    };
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='60px'>
          <HBoxLayout width='200px' height='60px' className={
            css(DashboardPage.STYLE.header)
          }>
            <Transition in={this.state.isSideMenuOpen}
                timeout={DashboardPage.MENU_TRANSITION_LENGTH_MS}
                unmountOnExit={true} component={null}>
              {(status: string) => {
                return (
                <HBoxLayout width='200px' height='60px' className={
                    css([DashboardPage.STYLE.menuIsOpenHeaderStyle,
                      headerStyle(status)])}>
                  <Padding size='18px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='23px'/>
                    <BurgerButton width='20px' height='14px' color={buttonColor}
                      highlightColor={buttonHighlightColor}
                      onClick={this.toggleSideMenuIsOpen}/>
                    <Padding size='23px'/>
                  </VBoxLayout>
                  <Padding size='20px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='15px'/>
                    <img src={logoImageSrc} width='78px' height='30px'/>
                    <Padding size='15px'/>
                  </VBoxLayout>
                </HBoxLayout>);
              }}
            </Transition>
            <Transition in={!this.state.isSideMenuOpen}
                timeout={DashboardPage.MENU_TRANSITION_LENGTH_MS}
                unmountOnExit={true} component={null}>
              {(status: string) => {
                return (
                <HBoxLayout width='200px' height='60px' className={
                    css([DashboardPage.STYLE.menuIsNotOpenHeaderStyle,
                      headerStyle(status)])}>
                  <Padding size='18px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='23px'/>
                    <BurgerButton width='20px' height='14px' color={buttonColor}
                      highlightColor={buttonHighlightColor}
                      onClick={this.toggleSideMenuIsOpen}/>
                    <Padding size='23px'/>
                  </VBoxLayout>
                  <Padding size='20px'/>
                  <VBoxLayout height='60px'>
                    <Padding size='15px'/>
                    <img src={logoImageSrc} width='78px' height='30px'/>
                    <Padding size='15px'/>
                  </VBoxLayout>
                </HBoxLayout>);
              }}
            </Transition>
          </HBoxLayout>
          <Padding/>
          <VBoxLayout height='60px' width='45px'>
            <Padding size='20px'/>
            <NotificationButton items={0} isOpen={false}/>
            <Padding size='20px'/>
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
        <div className={css(DashboardPage.STYLE.separator)}/>
        <Transition in={this.state.isSideMenuOpen}
            timeout={DashboardPage.MENU_TRANSITION_LENGTH_MS}
            unmountOnExit={true} component={null}>
          {(status: string) => {
            return (
              <HBoxLayout height='100%' className={sideMenuClassName(status)}>
                <SideMenu roles={this.props.model.roles}/>
              </HBoxLayout>);
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
      backgroundColor: 'rgba(0, 0, 0, 0.16)'
    },
    header: {
      position: 'absolute' as 'absolute',
      top: 0,
      left: 0,
    },
    menuIsOpenHeaderStyle: {
      opacity:  0,
      transition: `opacity ${DashboardPage.MENU_TRANSITION_LENGTH_MS}ms `
        + `ease-out`,
      backgroundColor: '#4B23A0'
    },
    menuIsNotOpenHeaderStyle: {
      position: 'absolute' as 'absolute',
      opacity:  0,
      transition: `opacity ${DashboardPage.MENU_TRANSITION_LENGTH_MS}ms `
        + `ease-out`,
      backgroundColor: '#FFFFFF'
    }
  });
  private static SIDE_MENU_TRANSITION_STYLE = StyleSheet.create({
    base: {
      opacity:  0,
      transition: `opacity ${DashboardPage.MENU_TRANSITION_LENGTH_MS}ms `
        + `ease-out`
    },
    entering: {opacity:  0},
    entered: {opacity:  1},
    exiting: {opacity:  0}
  });
  private static HEADER_TRANSITION_STYLE = StyleSheet.create({
    base: {
      opacity:  0,
      transition: `opacity ${DashboardPage.MENU_TRANSITION_LENGTH_MS}ms `
        + `ease-out`
    },
    entering: {opacity:  0},
    entered: {opacity:  1},
    exiting: {opacity:  0},
    exited: {opacity:  0}
  });
}
