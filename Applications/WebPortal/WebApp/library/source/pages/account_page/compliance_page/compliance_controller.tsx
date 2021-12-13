import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { LoadingPage, LoadingState } from '../..';

interface Properties {}

interface State {
  loadingState: LoadingState;
}

/** Implements the controller for the CompliancePage. */
export class ComplianceController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      loadingState: new LoadingState()
    }
  }

  public render(): JSX.Element {
    if(this.state.loadingState.isLoading()) {
      return <LoadingPage/>;
    }
    return <div/>;
  }
}
