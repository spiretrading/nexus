import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { CreateAccountPage } from './create_account_page';

interface Properties {
  /** The size of the viewport. */
  displaySize: DisplaySize;
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
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <div/>;
    }
    return <CreateAccountPage
      displaySize={this.props.displaySize}
      errorStatus={''}
      countryDatabase={}
      groupSuggestionModel={}
      onSubmit={}/>;
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoaded: true
        });
      });
  }

  private onSubmit(username: string, groups: Beam.DirectoryEntry[],
    identity: Nexus.AccountIdentity, roles: Nexus.AccountRoles): void {
      console.log('do a submit');
  }
}
