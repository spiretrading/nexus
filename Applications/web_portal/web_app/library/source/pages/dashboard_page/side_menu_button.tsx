import * as React from 'react';

/** Specifies the properties used to render a SideMenuButton. */
export interface Properties {

  /** The text label. */
  label: string;

  /** The icon to display. */
  icon: string;

  /** The action to perform. */
  onClick?: () => void;
}

export interface State {}

/** Renders a single SideMenu button. */
export class SideMenuButton extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return null;
  }
}
