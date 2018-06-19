import * as React from 'react';

/** The properties used to display the BurgerButton. */
export interface Properties {

  /** The width of the button. */
  width: number | string;

  /** The height of the button. */
  height: number | string;

  /** The color of the bars. */
  color: string;

  /** The color of the bars when highlighted. */
  highlightColor: string;

  /** The onClick event handler. */
  onClick?: () => void;
}

export interface State {}

/** Displays a burger button. */
export class BurgerButton extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    return null;
  }
}
