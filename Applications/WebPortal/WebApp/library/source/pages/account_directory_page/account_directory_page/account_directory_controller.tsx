import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { DisplaySize, LoadingPage } from '../../..';
import { CreateAccountController } from '../create_account_page';
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
  isLoaded: boolean;
  openedGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  filter: string;
  filteredGroups: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  createGroupStatus: string;
  redirect: string;
}

/** Implements the controller for the AccountDirectoryPage. */
export class AccountDirectoryController extends
    React.Component<Properties, State> {
  public static readonly defaultProps = {
    urlPrefix: ''
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false, 
      openedGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>(),
      filter: '',
      filteredGroups: new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>(),
      createGroupStatus: '',
      redirect: '/account_directory'
    };
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return (
      <Router.Switch>
        <Router.Route path='/create_account'
          render={this.renderCreateAccountPage}/>
        <Router.Route path='/account_directory'
          render={this.renderAccountPage}/>
      </Router.Switch>);
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true
        });
      });
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private renderAccountPage = () => {
    return <AccountDirectoryPage
      displaySize={this.props.displaySize}
      roles={this.props.roles}
      groups={this.props.model.groups}
      openedGroups={this.state.openedGroups}
      filter={this.state.filter}
      filteredGroups={this.state.filteredGroups}
      onFilterChange={this.onFilterChange} 
      onCardClick={this.onCardClick}
      createGroupStatus={this.state.createGroupStatus}
      onCreateGroup={this.onCreateGroup}
      onNewAccountClick={this.onNewAccountClick}/>;
  }

  private renderCreateAccountPage = () => {
    return <CreateAccountController 
      displaySize={this.props.displaySize}
      countryDatabase={this.props.countryDatabase}
      createAccountModel={this.props.model.createAccountModel}
      groupSuggestionModel={this.props.model.groupSuggestionModel}/>;
  }

  private onCardClick = async (group: Beam.DirectoryEntry) => {
    if(this.state.openedGroups.get(group)) {
      this.state.openedGroups.remove(group);
    } else {
      const accounts = await this.props.model.loadAccounts(group);
      this.state.openedGroups.set(group, accounts);
    }
    this.setState({openedGroups: this.state.openedGroups});
  }

  private onCreateGroup = async (name: string) => {
    try {
      await this.props.model.createGroup(name);
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
      this.setState({filter: newFilter, filteredGroups: accounts});
    } else {
      this.setState({filter: newFilter});
    }
  }
}
