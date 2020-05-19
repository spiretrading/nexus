import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  errorStatus: string;
  displayStatus: string;
  groups: Beam.DirectoryEntry[];
  model: WebPortal.GroupSuggestionModel;
}

/**  Displays and tests the CreateAccountPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      errorStatus: '',
      displayStatus: '',
      groups: [],
      model: new WebPortal.LocalGroupSuggestionModel([])
    };
    this.setErrorMessage = this.setErrorMessage.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
  }

  public render(): JSX.Element {
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
      <WebPortal.CreateAccountPage
        onSubmit={this.onSubmit}
        countryDatabase={this.someDB}
        displaySize={this.props.displaySize}
        errorStatus={this.state.displayStatus}
        groupSuggestionModel={this.state.model}/>
      <div style={TestApp.STYLE.testingComponents}>
        <button tabIndex={-1}
            onClick={() => this.setErrorMessage('Server issue')}>
          SERVER ERROR
        </button>
        <button tabIndex={-1}
            onClick={() => this.setErrorMessage('')}>
          NO SERVER ERROR
        </button>
      </div>
      </Dali.VBoxLayout>);
  }

  public componentDidMount(): void {
    const type = Beam.DirectoryEntry.Type.DIRECTORY;
    const group1 =
      new Beam.DirectoryEntry(type, 80, 'Lonely Mountain');
    const group2 =
      new Beam.DirectoryEntry(type, 41, 'Hobbits of Buckland');
    const group3 =
      new Beam.DirectoryEntry(type, 42, 'Mountains of Mirkwood');
    const group4 =
      new Beam.DirectoryEntry(type, 43, 'Hobbits of Shire');
    const group5 =
      new Beam.DirectoryEntry(type, 45, 'Mountains of Angmar');
    const group6 =
      new Beam.DirectoryEntry(type, 46, 'Hobbits of Bree');
    const group7 =
      new Beam.DirectoryEntry(type, 301, 'spire ontario');
    const group8 =
      new Beam.DirectoryEntry(type, 31, 'Spire United Kingdom');
    const group9 =
      new Beam.DirectoryEntry(type, 32, 'Spire China');
    const group10 =
      new Beam.DirectoryEntry(type, 33, 'Spire Japan');
    const group11 =
      new Beam.DirectoryEntry(type, 34, 'Spire Spain');
    const group12 =
      new Beam.DirectoryEntry(type, 35, 'Spire Singapore');
    const group13 =
      new Beam.DirectoryEntry(type, 302, 'spire quebec');
    const group14 =
      new Beam.DirectoryEntry(type, 37, 'Spire France');
    const group15 =
      new Beam.DirectoryEntry(type, 38, 'Spire Fiji');
    const group16 =
      new Beam.DirectoryEntry(type, 39, 'Spire Sweden');
    const group17 =
      new Beam.DirectoryEntry(type, 303, 'spire alberta');
    const group18 =
      new Beam.DirectoryEntry(type, 304, 'spire yukon');
    const group19 =
      new Beam.DirectoryEntry(type, 305, 'spire manitoba');
    const group20 =
      new Beam.DirectoryEntry(type, 306, 'spire nova scotia');
    const group21 =
      new Beam.DirectoryEntry(type, 307, 'spire new brunswick');
    const group22 =
      new Beam.DirectoryEntry(type, 308, 'spire british columbia');
    const group23 =
      new Beam.DirectoryEntry(type, 309, 'spire saskatchewan');
    this.state.groups.push(group1);
    this.state.groups.push(group2);
    this.state.groups.push(group3);
    this.state.groups.push(group4);
    this.state.groups.push(group5);
    this.state.groups.push(group6);
    this.state.groups.push(group7);
    this.state.groups.push(group8);
    this.state.groups.push(group9);
    this.state.groups.push(group10);
    this.state.groups.push(group11);
    this.state.groups.push(group12);
    this.state.groups.push(group13);
    this.state.groups.push(group14);
    this.state.groups.push(group15);
    this.state.groups.push(group16);
    this.state.groups.push(group17);
    this.state.groups.push(group18);
    this.state.groups.push(group19);
    this.state.groups.push(group20);
    this.state.groups.push(group21);
    this.state.groups.push(group22);
    this.state.groups.push(group23);
    const newModel = new WebPortal.LocalGroupSuggestionModel(this.state.groups);
    this.setState({
      model: newModel
    });
  }

  private onSubmit(username: string, groups: Beam.DirectoryEntry,
      identity: Nexus.AccountIdentity, roles: Nexus.AccountRoles) {
    this.setState({displayStatus: this.state.errorStatus});
  }

  private setErrorMessage(newValue: string) {
    this.setState({errorStatus: newValue});
  }

  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 1
    }
  };
  private someDB = Nexus.buildDefaultCountryDatabase();
}

const ResponsivePage =
  WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>,  document.getElementById('main'));
