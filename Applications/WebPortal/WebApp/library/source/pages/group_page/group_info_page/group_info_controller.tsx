import * as React from 'react';
import { AccountEntry, DisplaySize, LoadingPage } from '../../..';
import { GroupInfoPage } from './group_info_page';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The model representing the group. */
  accounts: AccountEntry[];
}

interface State {
  isLoaded: boolean;
  accounts: AccountEntry[];
}

/** Controller for the GroupInfoPage */
export class GroupInfoController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      accounts: []
    }
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return(
      <GroupInfoPage
        displaySize={this.props.displaySize}
        group={this.state.accounts}/>);
  }

  public componentDidMount(): void {
    this.setState({
      isLoaded: true,
      accounts: this.props.accounts.sort(GroupInfoController.accountComparator)
    });
  }

  private static accountComparator(accountA: AccountEntry,
      accountB: AccountEntry): number {
    return accountA.account.name.localeCompare(accountB.account.name);
  }
}
