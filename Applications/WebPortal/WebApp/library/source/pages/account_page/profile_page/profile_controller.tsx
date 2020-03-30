import * as React from 'react';

interface Properties {}

interface State {}

/** Implements a controller for the ProfilePage. */
export class ProfileController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {};
  }

  public render(): JSX.Element {
    return <div>Profile Page</div>;
  }
}
