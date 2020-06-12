import * as React from 'react';
import { AccountEntry, DisplaySize, LoadingPage } from '../../..';
import { GroupInfoModel } from './group_info_model';
import { GroupInfoPage } from './group_info_page';

interface Properties {

  /** The size of the viewport. */
  displaySize: DisplaySize;

  /** The model representing the group. */
  model: GroupInfoModel;
}

interface State {
  isLoaded: boolean;
  sortedAccounts: AccountEntry[];
}

/** Controller for the GroupInfoPage */
export class GroupInfoController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      sortedAccounts: []
    }
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return(
      <GroupInfoPage
        displaySize={this.props.displaySize}
        group={this.state.sortedAccounts}/>);
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true,
          sortedAccounts: this.props.model.group.sort(
            GroupInfoController.accountComparator)
        });
      });
  }

  public componentDidUpdate(prevProps: Properties): void {
    if(prevProps.model.group.toString() !== this.props.model.group.toString()) {
      this.setState({sortedAccounts: this.props.model.group.sort(
        GroupInfoController.accountComparator)});
    }
  }

  private static accountComparator(accountA: AccountEntry,
      accountB: AccountEntry): number {
    return accountA.account.name.localeCompare(accountB.account.name);
  }
}
