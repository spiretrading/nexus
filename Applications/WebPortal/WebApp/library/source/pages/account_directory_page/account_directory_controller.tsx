import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../..';
import { LoadingPage } from '..';
import { AccountDirectoryModel } from '.';
import { AccountDirectoryPage } from '.';

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
    if(!this.state.isLoaded) {
    return <LoadingPage/>;
    }
  }
}
