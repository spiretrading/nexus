import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  roles: Nexus.AccountRoles;
  groups: Beam.Set<Beam.DirectoryEntry>;
  openedGroups: Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>;
  filter: string;
  filteredGroups: Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>;
  model: WebPortal.AccountDirectoryModel;
  statusCreateGroup: string;
}

/**  Displays and tests the AccountDirectoryPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      roles: this.testAdmin,
      groups: new Beam.Set<Beam.DirectoryEntry>(),
      openedGroups: new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>(),
      filter: '',
      filteredGroups: new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>(),
      model: new WebPortal.LocalAccountDirectoryModel(
        new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>()),
      statusCreateGroup: ''
    };
    this.changeRole = this.changeRole.bind(this);
    this.onCardClick = this.onCardClick.bind(this);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const errorButtonText = (() => {
      if(this.state.statusCreateGroup === '') {
        return 'NO ERROR';
      } else {
        return 'ERROR'
      }
    })();
    return (
      <Router.BrowserRouter>
        <WebPortal.AccountDirectoryPage
          displaySize={this.props.displaySize}
          roles={this.state.roles}
          groups={this.state.groups}
          openedGroups={this.state.openedGroups}
          filter={this.state.filter}
          filteredGroups={this.state.filteredGroups}
          onFilterChange={this.onChange}
          onCardClick={this.onCardClick}
          createGroupStatus={this.state.statusCreateGroup}
          onCreateGroup={this.onCreateNewGroup}/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1}
              onClick={() =>
                this.changeRole(Nexus.AccountRoles.Role.ADMINISTRATOR)}>
            ADMINISTRATOR
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeRole(Nexus.AccountRoles.Role.TRADER)}>
            TRADER
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeRole(Nexus.AccountRoles.Role.MANAGER)}>
            MANAGER
          </button>
          <button tabIndex={-1} onClick={this.onToggleError}>
            {errorButtonText}
          </button>
        </div>
      </Router.BrowserRouter>
    );
  }

  public componentDidMount() {
    const accounts = 
      new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>();
    this.testAdmin.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    this.testTrader.set(Nexus.AccountRoles.Role.TRADER);
    this.testManager.set(Nexus.AccountRoles.Role.MANAGER);
    const group1 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 80, 'Nexus');
    const group2 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 41, 'Spire');
    const group3 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 31, 'Office');
    const group4 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 36, 'Shire');
    const group5 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 33, 'Mordor');
    const group6 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 37, 'Bree');
    this.state.groups.add(group1);
    this.state.groups.add(group2);
    this.state.groups.add(group3);
    this.state.groups.add(group4);
    this.state.groups.add(group5);
    this.state.groups.add(group6);
    const accountEntry1 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 9123, 'administration_service'),
      new Nexus.AccountRoles());
    const accountEntry2 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 23, 'daily_service'),
      new Nexus.AccountRoles());
    const accountEntry3 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 223, 'market_data_relay_service'),
      new Nexus.AccountRoles(3));
    const accountEntry4 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 45, 'data_relay_service'),
      new Nexus.AccountRoles());
    const accountEntry5 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 788, 'execution_service'),
      new Nexus.AccountRoles(5));
    const accountEntry6 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 1, 'data_news_service'),
      new Nexus.AccountRoles());
    const accountEntry7 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 5, 'data_relay_news'),
      new Nexus.AccountRoles());
    const testArray = [];
    testArray.push(accountEntry1);
    testArray.push(accountEntry2);
    testArray.push(accountEntry3);
    testArray.push(accountEntry4);
    testArray.push(accountEntry5);
    testArray.push(accountEntry6);
    testArray.push(accountEntry7);
    for(const group of this.state.groups) {
      if(group.id % 2 === 0) {
        accounts.set(group, testArray);
      } else {
        accounts.set(group, []);
      }
    }
    const testModel = new WebPortal.LocalAccountDirectoryModel(accounts);
    testModel.load();
    const newModel = new WebPortal.CachedAccountDirectoryModel(testModel);
    this.setState({model: newModel});
  }

  private changeRole(newRole: Nexus.AccountRoles.Role): void {
    if(newRole === Nexus.AccountRoles.Role.ADMINISTRATOR) {
      this.setState({roles: this.testAdmin, statusCreateGroup: ''});
    }
    if(newRole === Nexus.AccountRoles.Role.TRADER) {
      this.setState({roles: this.testTrader, statusCreateGroup: ''});
    }
    if(newRole === Nexus.AccountRoles.Role.MANAGER) {
      this.setState({ roles: this.testManager, statusCreateGroup: ''});
    }
  }

  private onChange(newFilter: string) {
    clearTimeout(this.timerId);
    this.setState({filter: newFilter});
    if(newFilter !== '') {
      this.timerId = setTimeout(
        async () => {
          const newAccounts =
            await this.state.model.loadFilteredAccounts(newFilter);
          this.setState({
            filteredGroups: newAccounts
          });
        }, 400);
    }
  }

  private onToggleError = () => {
    if(this.state.statusCreateGroup === '') {
      this.setState({statusCreateGroup: 'Server Issue'});
    } else {
      this.setState({statusCreateGroup: ''});
    }
  }

  private onCreateNewGroup = (groupName: string) => {
    if(this.state.statusCreateGroup === '') {
      clearTimeout(this.timerId);
      this.timerId = setTimeout(
        async () => {
          const newGroup = await this.state.model.createGroup(groupName);
          this.state.groups.add(newGroup);
          this.setState({groups: this.state.groups});
        }, 100);
    }
  }

  private async onCardClick(group: Beam.DirectoryEntry) {
    if(this.state.openedGroups.get(group)) {
      this.state.openedGroups.remove(group);
    } else {
      const accounts = await this.state.model.loadAccounts(group);
      this.state.openedGroups.set(group, accounts);
    }
    this.setState({openedGroups: this.state.openedGroups});
  }

  private testAdmin = new Nexus.AccountRoles();
  private testTrader = new Nexus.AccountRoles();
  private testManager = new Nexus.AccountRoles();
  private timerId: any; 
  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      fontSize: '8px',
      top: 0,
      left: 0,
      zIndex: 500
    }
  };
}

const ResponsivePage =
  WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
