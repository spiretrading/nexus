import * as React from 'react';
import { DisplaySize } from '../../display_size';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;
}

/** Displays an account's profile page. */
export class AccountDirectoryPage extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div>
        BEEP
      </div>);
  }
}
