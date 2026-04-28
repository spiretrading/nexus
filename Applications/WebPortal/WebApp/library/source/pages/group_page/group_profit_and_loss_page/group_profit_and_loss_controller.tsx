import * as React from 'react';
import { GroupProfitAndLossPage } from './group_profit_and_loss_page';

interface Properties {
}

interface State {
}

/** Implements a controller for the GroupProfitAndLossPage. */
export class GroupProfitAndLossController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {};
  }

  public render(): JSX.Element {
    return <GroupProfitAndLossPage/>;
  }
}
