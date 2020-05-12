import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, GroupSuggestionModel } from '../../..';
import { CreateAccountModel } from './create_account_model';
import { CreateAccountPage } from './create_account_page';

interface Properties {
  /** The size of the viewport. */
  displaySize: DisplaySize;
  
  /** The database of all available countries. */
  countryDatabase?: Nexus.CountryDatabase;

  /** The model used to create new accounts. */
  createAccountModel: CreateAccountModel;

  /** The model used to get group suggestions. */
  groupSuggestionModel: GroupSuggestionModel;
}

interface State {
  isLoaded: boolean;
  errorStatus: string;
}

/** Implements the controller for the AccountDirectoryPage. */
export class CreateAccountController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      errorStatus: ''
    };
    this.onSubmit = this.onSubmit.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <div/>;
    }
    return <CreateAccountPage
      displaySize={this.props.displaySize}
      errorStatus={this.state.errorStatus}
      countryDatabase={this.props.countryDatabase}
      groupSuggestionModel={this.props.groupSuggestionModel}
      onSubmit={this.onSubmit}/>;
  }

  public componentDidMount(): void {
    this.props.createAccountModel.load().then(
      () => {
        this.setState({
          isLoaded: true
        });
      });
  }

  private async onSubmit(username: string, groups: Beam.DirectoryEntry[],
      identity: Nexus.AccountIdentity, roles: Nexus.AccountRoles) {
    try {
      this.setState({
        errorStatus: ''
      });
      await this.props.createAccountModel.onSubmitNewAccount(roles, identity);
      this.setState({
        errorStatus: 'Created.'
      });
    } catch(e) {
      this.setState({
        errorStatus: e.toString()
      });
    }
  }
}
