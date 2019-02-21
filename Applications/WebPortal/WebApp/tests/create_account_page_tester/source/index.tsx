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
}

class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      errorStatus: '',
      displayStatus: ''
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
      />
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
