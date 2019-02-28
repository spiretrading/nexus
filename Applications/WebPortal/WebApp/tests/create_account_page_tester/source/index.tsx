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
}

/**  Displays displays and tests the CreateAccountPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      errorStatus: '',
      displayStatus: '',
      groups: []
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
        tempGroups={this.state.groups}/>
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

  public componentDidMount() {
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
    this.state.groups.push(group1);
    this.state.groups.push(group2);
    this.state.groups.push(group3);
    this.state.groups.push(group4);
    this.state.groups.push(group5);
    this.state.groups.push(group6);
  }

  private onSubmit(username: string, groups: Beam.DirectoryEntry[],
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
