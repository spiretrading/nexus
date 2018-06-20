import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..';
import {NotificationButton} from './notification_button';

export interface Properties {}

export interface State {}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%' height='100%'>
        <HBoxLayout width='100%' height='75px'
          className={css(DashboardPage.STYLE.header)}>
          <HBoxLayout width='136px' height='75px'>
            <Padding size='18px'/>
            <VBoxLayout height='75px'>
              <Padding/>
              <VBoxLayout height='30px'>
                <Padding size='7.5px'/>
                <BurgerButton width='20px' height='15px'
                  color='#4B23A0' highlightColor='#4B23A0'/>
                <Padding size='7.5px'/>
              </VBoxLayout>
              <Padding/>
            </VBoxLayout>
            <Padding size='20px'/>
            <VBoxLayout height='75px'>
              <Padding/>
              <img src='resources/dashboard/logo.png'
                className={css(DashboardPage.STYLE.logo)}/>
              <Padding/>
            </VBoxLayout>
          </HBoxLayout>
          <Padding/>
          <VBoxLayout height='75px' width='78px'>
            <Padding size='30px'/>
            <NotificationButton width='15px' height='20px'/>
            <Padding size='30px'/>
          </VBoxLayout>
        </HBoxLayout>
      </VBoxLayout>);
  }
  private static STYLE = StyleSheet.create({
    header: {
      boxShadow: '0 1px 0.5px -0.5px rgba(0, 0, 0, 0.16)'
    },
    logo: {
      width: '78px',
      height: '30px'
    }
  });
}
