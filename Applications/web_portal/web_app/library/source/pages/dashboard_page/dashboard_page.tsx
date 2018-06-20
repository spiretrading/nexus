import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..';

export interface Properties {}

export interface State {
  isSidebarOpen: boolean;
}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      isSidebarOpen: false
    }
    this.onBurgerButtonClick = this.onBurgerButtonClick.bind(this);
  }

  public render(): JSX.Element {
    const headerClassName = (() => {
      if(this.state.isSidebarOpen) {
        return css(DashboardPage.STYLE.highlightedHeader);
      }
      return css(DashboardPage.STYLE.defaultHeader);
    })();
    return(
      <VBoxLayout width='100%' height='100%'>
        <VBoxLayout height='60px' width='320px' className={headerClassName}>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='40px' height='34px' color='#684BC7'
                highlightColor='#FFFFFF' onClick={this.onBurgerButtonClick}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
      </VBoxLayout>);
  }

  private onBurgerButtonClick() {
    this.setState({isSidebarOpen: !this.state.isSidebarOpen});
  }
  private static STYLE = StyleSheet.create({
    defaultHeader: {
      backgroundColor: '#FFFFF'
    },
    highlightedHeader: {
      backgroundColor: '#4B23A0'
    }
  });
}
