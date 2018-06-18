
import * as React from 'react';

export interface Properties {}

export interface State {}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    return <div>Dashboard</div>
  }
}
