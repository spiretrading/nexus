import * as React from 'react';
import { DisplaySize } from '../../..';
import { FormEntry } from './form_entry';

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
    const orientation = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return FormEntry.Orientation.VERTICAL;
      } else {
        return FormEntry.Orientation.HORIZONTAL;
      }
    })();
    return <div/>;
  }
}
