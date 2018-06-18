import * as React from 'react';

/** The properties used to render a NotificationButton */
export interface Properties {

  /** The number of notification items. */
  items: number;

  /** Whether the button is in the open state (being viewed). */
  isOpen: boolean;
}

export interface State {}

/** Displays the notification button. */
export class NotificationButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    return null;
  }
}
