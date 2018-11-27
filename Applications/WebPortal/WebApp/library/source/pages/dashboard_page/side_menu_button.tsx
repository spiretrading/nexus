import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { Center, HBoxLayout, Padding, VBoxLayout } from '../..';

interface Properties {

  /** The text label. */
  label: string;

  /** The icon to display. */
  icon: string;

  /** Indicates the buttom was clicked. */
  onClick?: () => void;
}

/** Renders a single SideMenu button. */
export class SideMenuButton extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <button onClick={this.props.onClick}
          className={css(SideMenuButton.DYNAMIC_STYLES.button)}>
        <HBoxLayout style={SideMenuButton.STYLES.buttonContents}>
          <Padding size='18px'/>
          <Center width='20px' height='40px'>
            <img width='20px' height='20px' src={this.props.icon}/>
          </Center>
          <Padding size='20px'/>
          <VBoxLayout height='40px' style={SideMenuButton.STYLES.buttonText}>
            <Padding/>
            {this.props.label}
            <Padding/>
          </VBoxLayout>
        </HBoxLayout>
      </button>);
  }

  private static readonly STYLES = {
    buttonContents: {
      width: '100%',
      height: '40px',
      backgroundColor: 'inherit'
    },
    buttonText: {
      color: 'white',
      font: 'inherit',
      lineHeight: '20px'
    }
  }

  private static readonly DYNAMIC_STYLES = StyleSheet.create({
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
    }
  });
}
