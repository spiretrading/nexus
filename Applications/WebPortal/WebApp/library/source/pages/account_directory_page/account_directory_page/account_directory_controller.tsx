import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { DisplaySize, LoadingPage, LoadingState } from '../../..';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountDirectoryPage } from './account_directory_page';
import { AccountEntry } from './account_entry';

interface Properties {

  /** The device's display size. */
  displaySize: DisplaySize;

  /** The roles belonging to the account that's logged in. */
  roles: Nexus.AccountRoles;

  /** The database of countries. */
  countryDatabase: Nexus.CountryDatabase;

  /** The model representing the account directory. */
  model: AccountDirectoryModel;
}

interface State {
  redirect: string;
  loadingState: LoadingState;
  sortedKeys: Beam.DirectoryEntry[];
  filteredKeys: Beam.DirectoryEntry[];
  openedGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  filter: string;
  filteredGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  createGroupStatus: string;
}

/** Implements the controller for the AccountDirectoryPage. */
export class AccountDirectoryController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      redirect: null,
      loadingState: new LoadingState(),
      sortedKeys: [],
      filteredKeys: [],
      openedGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>(),
      filter: '',
      filteredGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>(),
      createGroupStatus: ''
    };
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    } else if(this.state.loadingState.isLoading()) {
      return <LoadingPage/>;
    } else if(this.state.loadingState.state === LoadingState.State.ERROR) {
      return <div/>;
    }
    const groups = (() => {
      if(this.state.filter !== '') {
        return this.state.filteredKeys;
      } else {
        return this.state.sortedKeys;
      }
    })();
    return <AccountDirectoryPage displaySize={this.props.displaySize}
      roles={this.props.roles} groups={groups}
      openedGroups={this.state.openedGroups} filter={this.state.filter}
      filteredGroups={this.state.filteredGroups}
      onFilterChange={this.onFilterChange}  onCardClick={this.onCardClick}
      createGroupStatus={this.state.createGroupStatus}
      onCreateGroup={this.onCreateGroup}
      onNewAccountClick={this.onNewAccountClick}/>;
  }

  public async componentDidMount(): Promise<void> {
    try {
      await this.props.model.load();
      this.setState(state => {
        return {
          loadingState: state.loadingState.succeed(),
          sortedKeys: this.props.model.groups.sort(
            AccountDirectoryController.groupComparator)
        };
      });
    } catch(error) {
      this.setState(state => {
        return {
          loadingState: state.loadingState.fail(error.toString())
        };
      });
    }
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private onCardClick = async (group: Beam.DirectoryEntry) => {
    if(this.state.openedGroups.get(group)) {
      this.state.openedGroups.remove(group);
    } else {
      const accounts = await this.props.model.loadAccounts(group);
      accounts.sort(AccountDirectoryController.accountComparator);
      this.state.openedGroups.set(group, accounts);
    }
    this.setState({openedGroups: this.state.openedGroups});
  }

  private onCreateGroup = async (name: string) => {
    try {
      await this.props.model.createGroup(name);
      this.setState({
        sortedKeys: this.props.model.groups.sort(
          AccountDirectoryController.groupComparator)
      });
    } catch(e) {
      this.setState({createGroupStatus: e.toString()});
    }
  }

  private onNewAccountClick = () => {
    this.setState({
      redirect: '/create_account'
    });
  }

  private onFilterChange = async (newFilter: string) => {
    if(newFilter !== '') {
      const accounts = await this.props.model.loadFilteredAccounts(newFilter);
      let keys = [] as Beam.DirectoryEntry[];
      for(const pair of accounts) {
        pair[1].sort(AccountDirectoryController.accountComparator);
        keys.push(pair[0]);
      }
      keys.sort(AccountDirectoryController.groupComparator);
      this.setState({
        filter: newFilter,
        filteredGroups: accounts,
        filteredKeys: keys
      });
    } else {
      this.setState({
        filter: newFilter,
        filteredKeys: [],
        openedGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>()
      });
    }
  }

  private static groupComparator(groupA: Beam.DirectoryEntry,
      groupB: Beam.DirectoryEntry): number {
    return groupA.name.localeCompare(groupB.name);
  }

  private static accountComparator(accountA: AccountEntry,
      accountB: AccountEntry): number {
    return accountA.account.name.localeCompare(accountB.account.name);
  }
}
