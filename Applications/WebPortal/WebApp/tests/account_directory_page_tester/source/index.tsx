import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  groups: Beam.Set<Beam.DirectoryEntry>;
  accounts: Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>;
  model: WebPortal.LocalAccountDirectoryModel;
}

/**  Displays a testing application. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
     groups : new Beam.Set<Beam.DirectoryEntry>(),
     accounts: new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>(),
     model: new WebPortal.LocalAccountDirectoryModel(
       new Beam.Set<Beam.DirectoryEntry>(),
       new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>())
    };
  }

  public render(): JSX.Element {
    return (
      <WebPortal.AccountDirectoryPage
        displaySize={this.props.displaySize}
        model={this.state.model}/>
    );
  }
  public componentDidMount() {
    const group1 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 80, 'Nexus');
    const group2 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 41, 'Spire');
    const group3 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 31, 'Office');
    const group4 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 35, 'Shire');
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
      new Nexus.AccountRoles()
    );
    const accountEntry2 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 23, 'charting_service'),
      new Nexus.AccountRoles()
    );
    const accountEntry3 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 223, 'market_data_relay_service'),
      new Nexus.AccountRoles()
    );
    const accountEntry4 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 45, 'data_relay_service'),
      new Nexus.AccountRoles()
    );
    const accountEntry5 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 788, 'order_execution_service'),
      new Nexus.AccountRoles()
    );
    const testArray = [];
    testArray.push(accountEntry1);
    testArray.push(accountEntry2);
    testArray.push(accountEntry3);
    testArray.push(accountEntry4);
    testArray.push(accountEntry5);
    for(const group of this.state.groups) {
      if(group.id % 2 === 0) {
        this.state.accounts.set(group, testArray);
      } else {
        this.state.accounts.set(group, []);
      }
    }
    const testModel = new WebPortal.LocalAccountDirectoryModel(
      this.state.groups, this.state.accounts);
    testModel.load();
    this.setState({model: testModel});
  }
}

const ResponsivePage =
  WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
