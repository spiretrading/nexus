import * as React from 'react';
import { AccountModel } from './account_model';
import { AccountPage } from './account_page';
import { DisplaySize } from '../../display_size';
import { SubPage } from './sub_page';

interface Properties {

  /** Determines the layout to use based on the display device. */
  displaySize: DisplaySize;

  /** The model representing the account to display. */
  model: AccountModel;
}

interface State {
  isLoaded: boolean;
  subPage: SubPage;
}

/** Implements a controller for the AccountPage. */
export class AccountController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      subPage: SubPage.PROFILE
    };
    this.onMenuClick = this.onMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <div/>;
    }
    const page = ((): JSX.Element => {
      switch(this.state.subPage) {
        default:
          return null;
      }
    })();
    return (
      <AccountPage displaySize={this.props.displaySize}
          subPage={this.state.subPage} account={this.props.model.account}
          roles={this.props.model.roles} onMenuClick={this.onMenuClick}>
        {page}
      </AccountPage>);
  }

  public componentWillMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({isLoaded: true});
      });
  }

  private onMenuClick(subPage: SubPage) {
    this.setState({subPage: subPage});
  }
}
