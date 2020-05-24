import { css, StyleSheet } from 'aphrodite';
import { Center, HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import * as Router from 'react-router-dom';

interface Properties {

  /** The text label. */
  label: string;

  /** The icon to display. */
  icon: string;

  /** The URL that the hyperlink points to. */
  href?: string;

  /** Signals the button was clicked. */
  onClick?: () => void;
}

/** Renders a single SideMenu button. */
export class SideMenuButton extends React.Component<Properties> {
  public render(): JSX.Element {
    const inner = (
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
      </HBoxLayout>);
    if(this.props.href) {
      return (
        <Router.Link to={this.props.href} onClick={this.props.onClick}
            className={css(SideMenuButton.DYNAMIC_STYLES.button)}>
          {inner}
        </Router.Link>);
    }
    return (
      <button onClick={this.props.onClick}
          className={css(SideMenuButton.DYNAMIC_STYLES.button)}>
        {inner}
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
      textDecoration: 'none',
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
