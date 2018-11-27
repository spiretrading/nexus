import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The number of notification items. */
  items: number;

  /** Whether the button is in the open state (being viewed). */
  isOpen: boolean;
}

/** Displays a notification button. */
export class NotificationButton extends React.Component<Properties> {
  public constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const STYLE = StyleSheet.create({
      button: {
        width: '15px',
        height: '20px',
        border: 'none',
        outline: 0,
        padding: 0,
        backgroundColor: 'transparent',
        shapeRendering: 'geometricPrecision',
        '-webkit-tap-highlight-color': 'transparent',
        ':hover': {
          cursor: 'pointer'
        }
      }
    });
    return (
      <button className={css(STYLE.button)}>
        <img width='15px' height='20px'
          src='resources/dashboard/icons/notification-grey.svg'/>
      </button>);
  }
}
