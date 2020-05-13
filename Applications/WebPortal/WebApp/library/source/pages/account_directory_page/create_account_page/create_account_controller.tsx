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
  errorStatus: string;
}

/** Implements the controller for the CreateAccountPage. */
export class CreateAccountController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      errorStatus: ''
    };
  }

  public render(): JSX.Element {
    return <CreateAccountPage
      displaySize={this.props.displaySize}
      errorStatus={this.state.errorStatus}
      countryDatabase={this.props.countryDatabase}
      groupSuggestionModel={this.props.groupSuggestionModel}
      onSubmit={this.createAccount}/>;
  }

  private createAccount = async (username: string,
      groups: Beam.DirectoryEntry[], identity: Nexus.AccountIdentity,
      roles: Nexus.AccountRoles) => {
    try {
      this.setState({
        errorStatus: ''
      });
      await this.props.createAccountModel.createAccount(
        username, groups, identity, roles);
    } catch(e) {
      this.setState({
        errorStatus: e.toString()
      });
    }
  }
}
