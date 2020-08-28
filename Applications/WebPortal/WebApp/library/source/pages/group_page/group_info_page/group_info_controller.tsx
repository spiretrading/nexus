import * as React from 'react';
import { AccountEntry, DisplaySize } from '../../..';
import { GroupInfoPage } from './group_info_page';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The model representing the group. */
  accounts: AccountEntry[];
}

interface State {
  accounts: AccountEntry[];
}

/** Controller for the GroupInfoPage */
export class GroupInfoController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      accounts: this.props.accounts.sort(GroupInfoController.accountComparator)
    }
  }

  public render(): JSX.Element {
    return(
      <GroupInfoPage
        displaySize={this.props.displaySize}
        group={this.state.accounts}/>);
  }

  private static accountComparator(accountA: AccountEntry,
      accountB: AccountEntry): number {
    return accountA.account.name.localeCompare(accountB.account.name);
  }
}
