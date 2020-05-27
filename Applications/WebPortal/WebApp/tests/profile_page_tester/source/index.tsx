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
  account: Beam.DirectoryEntry;
  groups: Beam.DirectoryEntry[];
  countryDatabase: Nexus.CountryDatabase;
  hasPassword: boolean;
  readOnly: boolean;
  isPasswordSubmitEnabled: boolean;
  hasError: boolean;
  hasPasswordError: boolean;
  testProfileError: boolean;
  testPasswordError: boolean;
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
      groups: [],
      countryDatabase: Nexus.buildDefaultCountryDatabase(),
      hasPassword: true,
      readOnly: false,
      isPasswordSubmitEnabled: false,
      hasPasswordError: false,
      testProfileError: false,
      testPasswordError: false
    };
    this.initIdentity();
    this.setStatusToError = this.setStatusToError.bind(this);
    this.setStatusToNull = this.setStatusToNull.bind(this);
    this.setStatusToSuccessful = this.setStatusToSuccessful.bind(this);
    this.togglePasswordVisibility = this.togglePasswordVisibility.bind(this);
    this.toggleReadonlyOfForm = this.toggleReadonlyOfForm.bind(this);
    this.passwordSubmit = this.passwordSubmit.bind(this);
    this.profileSubmit = this.profileSubmit.bind(this);
  }

  public render(): JSX.Element {
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.ProfilePage
          account={this.state.account}
          roles={this.state.someRoles}
          identity={this.state.identity}
          groups={this.state.groups}
          countryDatabase={this.state.countryDatabase}
          displaySize={this.props.displaySize}
          readonly={this.state.readOnly}
          onSubmit={this.profileSubmit}
          submitStatus={this.state.statusMessage}
          hasError={this.state.hasError}
          hasPassword={this.state.hasPassword}
          submitPasswordStatus={this.state.passwordStatusMessage}
          hasPasswordError={this.state.hasPasswordError}
          onSubmitPassword={this.passwordSubmit}/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1}
            onClick={this.setStatusToNull}>
            NO STATUS MESSAGES
          </button>
          <button tabIndex={-1}
            onClick={this.setStatusToSuccessful}>
            SAVED FEEDBACK MESSAGES
          </button>
          <button tabIndex={-1}
            onClick={this.setStatusToError}>
            ERROR MESSAGES
          </button>
          <button tabIndex={-1}
            onClick={this.togglePasswordVisibility}>
            TOGGLE PASSWORD FIELD
          </button>
          <button tabIndex={-1}
            onClick={this.toggleReadonlyOfForm}>
            TOGGLE READONLY
          </button>
        </div>
      </Dali.VBoxLayout>);
  }

  private initIdentity() {
    this.state.identity.photoId = TestApp.SOME_IMAGE;
    this.state.identity.firstName = 'Frodo';
    this.state.identity.lastName = 'Baggins';
    this.state.identity.lastLoginTime = new Beam.DateTime(
      new Beam.Date(2018, Beam.Date.Month.DECEMBER, 20),
      Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
      Beam.Duration.SECOND.multiply(15)));
    this.state.identity.province = 'Westfarthing';
    this.state.identity.country = Nexus.DefaultCountries.AU;
    this.state.identity.city = 'Hobbiton';
    this.state.identity.addressLineOne = '56 Bag Ennd';
    this.state.identity.addressLineTwo = '';
    this.state.identity.addressLineThree = '3rd door on the left';
    this.state.identity.userNotes = '';
    this.state.identity.emailAddress = 'frodo@bagend.nz';
    this.state.identity.registrationTime = new Beam.DateTime(
      new Beam.Date(2017, Beam.Date.Month.DECEMBER, 21),
      Beam.Duration.HOUR.multiply(5).add(Beam.Duration.MINUTE.multiply(30)).add(
      Beam.Duration.SECOND.multiply(15)));
    this.state.groups.push(new Beam.DirectoryEntry(
      Beam.DirectoryEntry.Type.NONE, 18, 'shire_office'));
    this.state.groups.push(new Beam.DirectoryEntry(
      Beam.DirectoryEntry.Type.NONE, 19, 'bree_office'));
    this.setState({identity: this.state.identity});
  }

  private setStatusToNull() {
    this.setState({
      statusMessage: '',
      passwordStatusMessage: '',
      hasError: false,
      hasPasswordError: false
    });
  }

  private setStatusToError() {
    this.setState({
      testPasswordError: true,
      testProfileError: true
    });
  }

  private setStatusToSuccessful() {
    this.setState({
      testPasswordError: false,
      testProfileError: false
    });
  }

  private togglePasswordVisibility() {
    this.setState({
      hasPassword: !this.state.hasPassword,
      isPasswordSubmitEnabled: !this.state.isPasswordSubmitEnabled
    });
  }

  private toggleReadonlyOfForm() {
    this.setState({
      readOnly: !this.state.readOnly
    });
  }

  private passwordSubmit(newPassword: string) {
    if(this.state.testPasswordError) {
      this.setState({
        passwordStatusMessage: 'Password not saved',
        hasPasswordError: true
      });
    } else {
      this.setState({
        passwordStatusMessage: 'Saved',
        hasPasswordError: false
      });
    }
  }

  private profileSubmit(roles: Nexus.AccountRoles, identity: Nexus.AccountIdentity) {
    if(this.state.testProfileError) {
      this.setState({
        statusMessage: 'Profile not saved',
        hasError: true
      });
    } else {
      this.setState({
        statusMessage: 'Saved',
        hasError: false
      });
    }
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
