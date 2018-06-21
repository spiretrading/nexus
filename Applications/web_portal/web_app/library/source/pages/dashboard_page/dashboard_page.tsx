import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..';
import {NotificationButton} from './notification_button';

export interface Properties {}

export interface State {
}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%' height='100%'>
        <div className={css(DashboardPage.STYLE.header)}>
          <HBoxLayout className={css(DashboardPage.STYLE.leftHeader)}
              width='136px' height='75px'>
            <Padding size='18px'/>
            <VBoxLayout height='75px'>
              <Padding/>
              <VBoxLayout height='30px'>
                <Padding size='calc(50% - 15px)'/>
                <BurgerButton width='20px' height='15px'
                  color='#684BC7' highlightColor='#684BC7'/>
                <Padding size='calc(50% - 15px)'/>
              </VBoxLayout>
              <Padding/>
            </VBoxLayout>
            <Padding size='20px'/>
            <VBoxLayout height='75px'>
              <Padding size='calc(50% - 15px)'/>
              <img src='resources/dashboard/logo.png'
                className={css(DashboardPage.STYLE.logo)}/>
              <Padding size='calc(50% - 15px)'/>
            </VBoxLayout>
          </HBoxLayout>
          <VBoxLayout height='75px' width='45px'
              className={css(DashboardPage.STYLE.rightHeader)}>
            <Padding size='calc(50% - 15px)'/>
            <NotificationButton width='15px' height='20px'/>
            <Padding size='calc(50% - 15px)'/>
          </VBoxLayout>
        </div>
      </VBoxLayout>);
  }
  private static STYLE = StyleSheet.create({
    header: {
      width: '100%',
      height: '75px',
      backgroundColor: 'rgba(255, 255, 255, 0)',
      boxShadow: '0 1px 1px -1px rgba(0, 0, 0, 0.16)',
      '-moz-box-shadow': '0 1px 1px -1px rgba(0, 0, 0, 0.16)'
    },
    leftHeader: {
      position: 'absolute' as 'absolute',
      top: 0,
      left: 0
    },
    rightHeader: {
      position: 'absolute' as 'absolute',
      top: 0,
      right: 0
    },
    logo: {
      width: '78px',
      height: '30px'
    }
  });
}
