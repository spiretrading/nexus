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
              width='136px' height='60px'>
            <Padding size='18px'/>
            <VBoxLayout height='60px'>
              <Padding/>
              <div className={css(DashboardPage.STYLE.burgerButtonWrapper)}>
                <BurgerButton width='20px' height='14px'
                  color='#684BC7' highlightColor='#684BC7'/>
              </div>
              <Padding/>
            </VBoxLayout>
            <Padding size='20px'/>
            <VBoxLayout height='60px'>
              <Padding/>
              <div className={css(DashboardPage.STYLE.logo)}>
                <img src='resources/dashboard/logo.png'
                  className={css(DashboardPage.STYLE.logo)}/>
              </div>
              <Padding/>
            </VBoxLayout>
          </HBoxLayout>
          <VBoxLayout height='60px' width='45px'
              className={css(DashboardPage.STYLE.rightHeader)}>
            <Padding/>
            <div className={css(DashboardPage.STYLE.notificationButtonWrapper)}>
              <NotificationButton width='15px' height='20px'/>
            </div>
            <Padding/>
          </VBoxLayout>
        </div>
        <div className={css(DashboardPage.STYLE.separator)}/>
      </VBoxLayout>);
  }
  private static STYLE = StyleSheet.create({
    header: {
      width: '100%',
      height: '60px',
      backgroundColor: 'rgba(255, 255, 255, 0)'
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
    burgerButtonWrapper: {
      width: '20px',
      height: '14px'
    },
    logo: {
      width: '78px',
      height: '30px'
    },
    notificationButtonWrapper: {
      width: '15px',
      height: '20px'
    },
    separator: {
      width: '100%',
      height: '1px',
      backgroundColor: 'rgba(0, 0, 0, 0.16)'
    }
  });
}
