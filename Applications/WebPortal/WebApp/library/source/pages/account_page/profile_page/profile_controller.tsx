import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, LoadingPage } from '../../..'
import { ProfileModel } from './profile_model';
import { ProfilePage } from './profile_page';

interface Properties {


  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** The database of all available countries. */
  countryDatabase?: Nexus.CountryDatabase;

  /** The names of the groups the account belongs to. */
  groups: Beam.DirectoryEntry[];

  /** The model representing the account's profile. */
  model: ProfileModel;
}

interface State {
  isLoaded: boolean;
  isIdentityError: boolean;
  identityStatus: string;
  isPasswordError: boolean;
  passwordStatus: string;
}

/** Implements a controller for the ProfilePage. */
export class ProfileController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      isIdentityError: false,
      identityStatus: '',
      isPasswordError: false,
      passwordStatus: ''
    };
    this.onIdentitySubmit = this.onIdentitySubmit.bind(this);
    this.onPasswordSubmit = this.onPasswordSubmit.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return <ProfilePage 
        account={this.props.model.account}
        roles={this.props.model.roles}
        identity={this.props.model.identity}
        groups={this.props.groups}
        countryDatabase={this.props.countryDatabase}
        displaySize={this.props.displaySize}
        readonly={this.props.model.roles.test(
          Nexus.AccountRoles.Role.ADMINISTRATOR)}
        submitStatus={this.state.identityStatus}
        hasError={this.state.isIdentityError}
        onSubmit={this.onIdentitySubmit}/>;
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true
        });
      });
  }

  private async onPasswordSubmit(password: string) {
    try {
      this.setState({
        isPasswordError: false,
        passwordStatus: ''
      });
      await this.props.model.updatePassword(password);
      this.setState({
        passwordStatus: 'Saved.'
      });
    } catch(e) {
      this.setState({
        isPasswordError: true,
        passwordStatus: e.toString()
      });
    }
  }

  private async onIdentitySubmit(
      roles: Nexus.AccountRoles, identity: Nexus.AccountIdentity) {
    try {
      this.setState({
        isIdentityError: false,
        identityStatus: ''
      });
      await this.props.model.updateIdentity(roles, identity);
      this.setState({
        identityStatus: 'Saved.'
      });
    } catch(e) {
      this.setState({
        isIdentityError: true,
        identityStatus: e.toString()
      });
    }
  }
}
