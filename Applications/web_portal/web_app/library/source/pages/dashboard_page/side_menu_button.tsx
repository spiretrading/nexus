import * as React from 'react';

interface Properties {

  /** The text label. */
  label: string;

  /** The icon to display. */
  icon: string;

  /** The action to perform. */
  onClick?: () => void;
}

interface State {}

/** Renders a single SideMenu button. */
export class SideMenuButton extends React.Component<Properties, State> {
  public render(): JSX.Element {
    return null;
  }
}
