import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { Center, HBoxLayout, Padding, VBoxLayout } from '../..';

interface Properties {

  /** The text label. */
  label: string;

  /** The icon to display. */
  icon: string;

  /** The action to perform. */
  onClick?: () => void;
}

/** Renders a single SideMenu button. */
export class SideMenuButton extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <button onClick={this.props.onClick} className={
          css(SideMenuButton.STYLE.button)}>
        <HBoxLayout className={
            css(SideMenuButton.STYLE.buttonContents)}>
          <Padding size='18px'/>
          <Center width='20px' height='40px'>
            <img className={css(SideMenuButton.STYLE.img)}
              src={this.props.icon}/>
          </Center>
          <Padding size='20px'/>
          <VBoxLayout height='40px'
              className={css(SideMenuButton.STYLE.buttonText)}>
            <Padding/>
            {this.props.label}
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
      </button>);
  }
  private static STYLE = StyleSheet.create({
    button: {
      width: '200px',
      height: '40px',
      backgroundColor: '#4B23A0',
      outline: 0,
      border: 'none',
      padding: 0,
      font: '200 14px Roboto',
      '-webkit-tap-highlight-color': 'transparent',
      ':hover': {
        backgroundColor: '#684BC7',
        cursor: 'pointer',
        color: 'blue'
      }
    },
    buttonContents: {
      width: '100%',
      height: '40px',
      backgroundColor: 'inherit'
    },
    buttonText: {
      color: 'white',
      font: 'inherit',
      lineHeight: '20px'
    },
    img: {
      width: '20px',
      height: '20px'
    }
  });
}
