import * as Beam from 'beam';
import * as Dali from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

/** Determines the size to render components at. */
interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  someRoles: Nexus.AccountRoles;
  imageSource: string;
  identity: Nexus.AccountIdentity;
  statusMessage: string;
  passwordStatusMessage: string;
  hasError: boolean;
  account: Beam.DirectoryEntry;
  group: Beam.DirectoryEntry;
  countryDatabase: Nexus.CountryDatabase;
}

/**  Displays a testing application. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      someRoles: new Nexus.AccountRoles(),
      imageSource: TestApp.SOME_IMAGE,
      identity: new Nexus.AccountIdentity(),
      statusMessage: '',
      passwordStatusMessage: '',
      hasError: false,
      account: new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 9123, 'frodo_of_the_nine_fingers'),
      group: new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.NONE, 18, 'shire_office'),
      countryDatabase: Nexus.buildDefaultCountryDatabase()
    };
    this.setStatusToError = this.setStatusToError.bind(this);
    this.setStatusToNull = this.setStatusToNull.bind(this);
    this.setStatusToSuccessful = this.setStatusToSuccessful.bind(this);
  }

  public render(): JSX.Element {
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.ProfilePage
          roles={this.state.someRoles}
          identity={this.state.identity}
          displaySize={this.props.displaySize}
          isSubmitEnabled={true}
          submitStatus={this.state.statusMessage}
          submitPasswordStatus={this.state.passwordStatusMessage}
          hasError={this.state.hasError}
          hasPasswordError={this.state.hasError}
          account={this.state.account}
          group={this.state.group}
          countryDatabase={this.state.countryDatabase}/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1}
            onClick={this.setStatusToNull}>
            NO STATUS MESSAGES
          </button>
          <button tabIndex={-1}
            onClick={this.setStatusToSuccessful}>
            STATUS MESSAGES
          </button>
          <button tabIndex={-1}
            onClick={this.setStatusToError}>
            ERROR MESSAGES
          </button>
        </div>
      </Dali.VBoxLayout>);
  }

  public componentDidMount() {
    const testIdentity = this.state.identity.clone();
    testIdentity.photoId = TestApp.SOME_IMAGE;
    testIdentity.firstName = 'Frodo';
    testIdentity.lastName = 'Baggins';
    testIdentity.lastLoginTime = new Beam.DateTime(
      new Beam.Date(2018, Beam.Date.Month.DECEMBER, 20),
      Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
      Beam.Duration.SECOND.multiply(15)));
    testIdentity.province = 'Westfarthing';
    testIdentity.country = Nexus.DefaultCountries.AU;
    testIdentity.city = 'Hobbiton';
    testIdentity.addressLineOne = '56 Bag End';
    testIdentity.emailAddress = 'frodo@bagend.nz';
    testIdentity.registrationTime = new Beam.DateTime(
      new Beam.Date(2017, Beam.Date.Month.DECEMBER, 21),
      Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
      Beam.Duration.SECOND.multiply(15)));
    this.setState({identity: testIdentity});
  }

  private setStatusToNull() {
    this.setState({
      statusMessage: '',
      passwordStatusMessage: '',
      hasError: false
    });
  }

  private setStatusToError() {
    this.setState({
      statusMessage: 'Error!',
      passwordStatusMessage: 'Error!',
      hasError: true
    });
  }

  private setStatusToSuccessful() {
    this.setState({
      statusMessage: 'Saved',
      passwordStatusMessage: 'Saved',
      hasError: false
    });
  }

  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      fontSize: '8px',
      top: 0,
      left: 0,
      zIndex: 500
    }
  };
  private static readonly SOME_IMAGE = 'https://upload.wikimedia.org/' +
    'wikipedia/commons/thumb/2/23/Close_up_of_a_black_domestic_cat.jpg/' +
    '675px-Close_up_of_a_black_domestic_cat.jpg';
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
