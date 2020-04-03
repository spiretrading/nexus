import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, LoadingPage } from '../../..'
import { ProfileModel } from './profile_model';
import { ProfilePage } from './profile_page';
import { AccountRoles } from 'nexus';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

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
    const isReadonly = (() => {
      if(this.props.model.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
      } else {
        return false;
      }
    })();
    return <div/>
   /*<ProfilePage 
      account={this.props.model.account}
      roles={this.props.model.roles}
      identity={this.props.model.identity}
      groups={'missing'}
      countryDatabase={'missing'}
      displaySize={this.props.displaySize}
      readonly={isReadonly}
      isSubmitEnabled={isReadonly}
      submitStatus={this.state.identityStatus}
      hasError={this.state.isIdentityError}
      onSubmit={this.onIdentitySubmit}
       />;*/
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
