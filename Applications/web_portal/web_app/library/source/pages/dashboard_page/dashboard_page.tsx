import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
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
    const headerStyle = (() => {
      if(this.state.isSideMenuOpen) {
        return DashboardPage.STYLE.menuIsOpenHeaderStyle;
      }
      return DashboardPage.STYLE.menuIsNotOpenHeaderStyle;
    })();
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
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='60px'>
          <HBoxLayout width='200px' height='60px' className={css(headerStyle)}>
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
        {this.state.isSideMenuOpen ?
          <SideMenu roles={this.props.model.roles}/> :  null}
      </VBoxLayout>);
  }

  private toggleSideMenuIsOpen() {
    this.setState({
      isSideMenuOpen: !this.state.isSideMenuOpen
    });
  }

  private static STYLE = StyleSheet.create({
    separator: {
      width: '100%',
      height: '1px',
      backgroundColor: 'rgba(0, 0, 0, 0.16)'
    },
    menuIsOpenHeaderStyle: {
      backgroundColor: '#4B23A0'
    },
    menuIsNotOpenHeaderStyle: {
      backgroundColor: '#FFFFFF'
    }
  });
}
