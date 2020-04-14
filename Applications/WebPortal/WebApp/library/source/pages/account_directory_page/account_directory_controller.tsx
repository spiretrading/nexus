import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../..';
import { LoadingPage } from '..';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountDirectoryPage } from './account_directory_page';
import { AccountEntry } from './account_entry';

interface Properties {

  /** The device's display size. */
  displaySize: DisplaySize;

  /** The roles belonging to the account that's logged in. */
  roles: Nexus.AccountRoles;
  
  /** The model representing the account directory. */
  model: AccountDirectoryModel;
}

interface State {
  isLoaded: boolean;
  openedGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  filter: string;
  filteredGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}

/** Implements the controller for the AccountDirectoryPage. */
export class AccountDirectoryController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false, 
      openedGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>(),
      filter: '',
      filteredGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>()
    };
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    <AccountDirectoryPage
      displaySize={this.props.displaySize}
      roles={this.props.roles}
      groups={this.props.model.groups}
      openedGroups={this.state.openedGroups}
      filter={this.state.filter}
      filteredGroups={this.state.filteredGroups}
      onFilterChange={this.onFilterChange} 
      onCardClick={this.onCardClick}/>
  }

  private async onCardClick(group: Beam.DirectoryEntry) {
    if(this.state.openedGroups.get(group)) {
      this.state.openedGroups.remove(group);
    } else {
      this.timerId = setTimeout(
        async () => {
          const accounts = await this.props.model.loadAccounts(group);
          this.state.openedGroups.set(group, accounts);
        }, 400);
    }
    this.setState({openedGroups: this.state.openedGroups});
  }

  private async onFilterChange(newFilter: string) {
    clearTimeout(this.timerId);
    this.setState({filter: newFilter});
    if(newFilter !== '') {
      this.timerId = setTimeout(
        async () => {
          const newAccounts = 
            await this.props.model.loadFilteredAccounts(newFilter);
          this.setState({
            filteredGroups: newAccounts
          });
        }, 400);
    }
  }

  private timerId: NodeJS.Timer; 
}
