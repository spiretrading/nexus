import * as React from 'react';
import { DisplaySize } from '../../..';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;
}

/** Displays an account's profile page. */
export class ProfilePage extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return <div>Profile Page</div>;
  }
}
