import * as React from 'react';
import { DisplaySize, LoadingPage } from '../..';
import { AccountController, AccountDirectoryController, AccountModel }
  from '..';
import { DashboardModel } from './dashboard_model';
import { DashboardPage } from './dashboard_page';
import { SideMenu } from './side_menu';

interface Properties {

  /** The model to use. */
  model: DashboardModel;

  /** The device's display size. */
  displaySize: DisplaySize;

  /** Indicates the user has logged out. */
  onLogout?: () => void;
}

interface State {
  isLoaded: boolean;
  page: DashboardController.Page;
}

/** Implements the controller for the DashboardPage. */
export class DashboardController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      page: DashboardController.Page.ACCOUNT
    };
    this.onSideMenuClick = this.onSideMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    const page = (() => {
      switch(this.state.page) {
        case DashboardController.Page.ACCOUNT:
          return <AccountController
            entitlements={this.props.model.entitlementDatabase}
            currencyDatabase={this.props.model.currencyDatabase}
            marketDatabase={this.props.model.marketDatabase}
            model={this.accountModel} displaySize={this.props.displaySize}/>;
        case DashboardController.Page.DIRECTORY:
          return <AccountDirectoryController
            displaySize={this.props.displaySize}/>;
      }
    })();
    return <DashboardPage roles={this.props.model.roles}
      onSideMenuClick={this.onSideMenuClick}>{page}</DashboardPage>;
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.accountModel = this.props.model.makeAccountModel(
          this.props.model.account);
        this.setState({isLoaded: true});
      });
  }

  private onSideMenuClick(item: SideMenu.Item) {
    if(item === SideMenu.Item.SIGN_OUT) {
      this.props.model.logout().then(
        () => {
          this.props.onLogout();
        });
    } else if(item === SideMenu.Item.ACCOUNTS) {
      this.setState({page: DashboardController.Page.DIRECTORY});
    }
  }

  private accountModel: AccountModel;
}

export namespace DashboardController {

  /** Lists the different pages this controller can display. */
  export enum Page {

    /** The directory page. */
    DIRECTORY,

    /** The profile page. */
    ACCOUNT
  }
}
