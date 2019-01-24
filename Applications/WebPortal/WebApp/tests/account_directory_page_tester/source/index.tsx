import * as Beam from 'beam';
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
        onNewAccountClick={null}
        onNewGroupClick={null}
        displaySize={this.props.displaySize}
        model={this.state.model}/>
    );
  }
  public componentDidMount() {
    const group1 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 89, 'Nexus');
    const group2 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 42, 'Spire');
    const group3 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 34, 'Office');
    const group4 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 35, 'Shire');
    const group5 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 36, 'Mordor');
    const group6 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 37, 'Bree');
    this.state.groups.add(group1);
    this.state.groups.add(group2);
    this.state.groups.add(group3);
    this.state.groups.add(group4);
    this.state.groups.add(group5);
    this.state.groups.add(group6);

    const testModel = new WebPortal.LocalAccountDirectoryModel(
      this.state.groups, this.state.accounts);
    testModel.load();
    this.setState({model: testModel});
  }
}

const ResponsivePage =
  WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
