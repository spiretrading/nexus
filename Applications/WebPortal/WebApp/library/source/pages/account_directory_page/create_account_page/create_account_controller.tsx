import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
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
  isSubmitting: boolean;
  isDone: boolean;
}

/** Implements the controller for the CreateAccountPage. */
export class CreateAccountController extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      errorStatus: '',
      isSubmitting: false,
      isDone: false
    };
  }

  public render(): JSX.Element {
    if(this.state.isDone) {
      return <Router.Redirect to='/account_directory'/>;
    }
    return <CreateAccountPage
      displaySize={this.props.displaySize}
      errorStatus={this.state.errorStatus}
      isSubmitting={this.state.isSubmitting}
      countryDatabase={this.props.countryDatabase}
      groupSuggestionModel={this.props.groupSuggestionModel}
      onSubmit={this.createAccount}/>;
  }

  private createAccount = async (username: string,
      groups: Beam.DirectoryEntry, identity: Nexus.AccountIdentity,
      roles: Nexus.AccountRoles) => {
    try {
      this.setState({
        errorStatus: '',
        isSubmitting: true
      });
      await this.props.createAccountModel.createAccount(
        username, groups, identity, roles);
      this.setState({isSubmitting: false, isDone: true});
    } catch(e: any) {
      this.setState({
        errorStatus: toErrorStatus(e),
        isSubmitting: false
      });
    }
  }

  public static readonly REJECTED_MESSAGE = 'Invalid inputs';
  public static readonly UNAVAILABLE_MESSAGE = 'Server issue';
}

const BAD_REQUEST = 400;
const CONFLICT = 409;

function toErrorStatus(error: any): string {
  if(error?.code === CONFLICT || error?.code === BAD_REQUEST) {
    return CreateAccountController.REJECTED_MESSAGE;
  }
  return CreateAccountController.UNAVAILABLE_MESSAGE;
}
