import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..';
import {DashboardModel} from '.';
import {NotificationButton} from './notification_button';

/** The properties used to render the DashboardPage. */
export interface Properties {

  /** The model to display. */
  model: DashboardModel;

  /** The action to perform when logging out. */
  onLogout?: () => void;
}

export interface State {}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='60px'>
          <HBoxLayout width='136px' height='60px'>
            <Padding size='18px'/>
            <VBoxLayout height='60px'>
              <Padding size='23px'/>
              <BurgerButton width='20px' height='14px' color='#684BC7'
                highlightColor='#684BC7'/>
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
          <VBoxLayout height='60px' width='45px'>
            <Padding size='20px'/>
            <NotificationButton items={0} isOpen={false}/>
            <Padding size='20px'/>
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
        <div className={css(DashboardPage.STYLE.separator)}/>
      </VBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    separator: {
      width: '100%',
      height: '1px',
      backgroundColor: 'rgba(0, 0, 0, 0.16)'
    }
  });
}
