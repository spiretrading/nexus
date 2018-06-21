import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';

/** The properties used to display the BurgerButton. */
export interface Properties {

  /** The width of the button. */
  width: number | string;

  /** The height of the button. */
  height: number | string;

  /** The onClick event handler. */
  onClick?: () => void;
}

/** Displays a notification button. */
export class NotificationButton extends React.Component<Properties> {
  public render(): JSX.Element {
    const STYLE = StyleSheet.create({
      button: {
        border: 'none',
        outline: 0,
        padding: 0,
        backgroundColor: 'rgba(0, 0, 0, 0)',
        width: this.props.width,
        height: this.props.height,
        ':hover': {
          cursor: 'pointer'
        }
      }
    });
    return (
      <button className={css(STYLE.button)} onClick={this.onClick.bind(this)}>
        <img width={this.props.width} height={this.props.height}
             style={{border: '1px solid black'}}
             src='resources/dashboard/notification.svg'/>
      </button>);
  }

  private onClick() {
    if(this.props.onClick) {
      this.props.onClick();
    }
  }
}
