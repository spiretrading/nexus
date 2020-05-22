import * as React from 'react';
import * as Router from 'react-router-dom';
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
  redirect: string;
}

/** Implements the controller for the DashboardPage. */
export class DashboardController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      redirect: null
    };
    this.onSideMenuClick = this.onSideMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return (
      <DashboardPage roles={this.props.model.roles}
          onSideMenuClick={this.onSideMenuClick}>
        <Router.Switch>
          <Router.Route path='*/account' render={() =>
            <AccountController
              entitlements={this.props.model.entitlementDatabase}
              countryDatabase={this.props.model.countryDatabase}
              currencyDatabase={this.props.model.currencyDatabase}
              marketDatabase={this.props.model.marketDatabase}
              model={this.accountModel}
              displaySize={this.props.displaySize}/>}/>
          <Router.Route path='*/account_directory' render={() =>
            <AccountDirectoryController
              displaySize={this.props.displaySize}
              roles={this.props.model.roles}
              countryDatabase={this.props.model.countryDatabase}
              model={this.props.model.accountDirectoryModel}/>}/>
          <Router.Route>
            <Router.Redirect to='/account'/>
          </Router.Route>
        </Router.Switch>
      </DashboardPage>);
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.accountModel = this.props.model.makeAccountModel(
          this.props.model.account);
        this.setState({isLoaded: true});
      });
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private onSideMenuClick(item: SideMenu.Item) {
    if(item === SideMenu.Item.PROFILE) {
      this.setState({redirect: '/account'});
    } else if(item === SideMenu.Item.ACCOUNTS) {
      this.setState({redirect: '/account_directory'});
    } else if(item === SideMenu.Item.SIGN_OUT) {
      this.props.model.logout().then(this.props.onLogout);
    }
  }

  private accountModel: AccountModel;
}
