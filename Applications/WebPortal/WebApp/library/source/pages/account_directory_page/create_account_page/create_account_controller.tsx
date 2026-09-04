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
  isDone: boolean;
}

/** Implements the controller for the CreateAccountPage. */
export class CreateAccountController extends
    React.Component<Properties, State> {
  public constructor(props: Properties) {
    super(props);
    this.state = {isDone: false};
  }

  public render(): JSX.Element {
    if(this.state.isDone) {
      return <Router.Redirect to='/account_directory'/>;
    }
    return <CreateAccountPage
      displaySize={this.props.displaySize}
      countryDatabase={this.props.countryDatabase}
      model={this.props.createAccountModel}
      groupSuggestionModel={this.props.groupSuggestionModel}
      onComplete={this.onComplete}/>;
  }

  private onComplete = () => {
    this.setState({isDone: true});
  }
}
