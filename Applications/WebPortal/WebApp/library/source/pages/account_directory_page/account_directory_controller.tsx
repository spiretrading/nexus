import * as React from 'react';
import { DisplaySize } from '../..';
import { LoadingPage } from '..';

interface Properties {

  /** The device's display size. */
  displaySize: DisplaySize;
}

interface State {
  isLoaded: boolean;
}

/** Implements the controller for the AccountDirectoryPage. */
export class AccountDirectoryController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false
    };
  }

  public render(): JSX.Element {
    return <LoadingPage/>;
  }
}
