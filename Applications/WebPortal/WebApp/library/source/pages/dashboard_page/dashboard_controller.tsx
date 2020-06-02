import * as Beam from 'beam';
import * as React from 'react';
import * as Router from 'react-router-dom';
import * as Path from 'path-to-regexp';
import { DisplaySize, LoadingPage } from '../..';
import { AccountController, AccountDirectoryController,
  CreateAccountController } from '..';
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
}

/** Implements the controller for the DashboardPage. */
export class DashboardController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
    };
    this.onSideMenuClick = this.onSideMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return (
      <DashboardPage roles={this.props.model.roles}
          onSideMenuClick={this.onSideMenuClick}>
        <Router.Switch>
          <Router.Route path='/account' render={this.renderAccountPage}/>
          <Router.Route path='/account_directory'
            render={() =>
              <AccountDirectoryController
                displaySize={this.props.displaySize}
                roles={this.props.model.roles}
                countryDatabase={this.props.model.countryDatabase}
                model={this.props.model.accountDirectoryModel}/>}/>
          <Router.Route path='/create_account'
            render={() =>
              <CreateAccountController 
                displaySize={this.props.displaySize}
                countryDatabase={this.props.model.countryDatabase}
                createAccountModel={
                  this.props.model.accountDirectoryModel.createAccountModel}
                groupSuggestionModel={
                  this.props.model.accountDirectoryModel.groupSuggestionModel}
                />}/>
          <Router.Route>
            <Router.Redirect to='/account'/>
          </Router.Route>
        </Router.Switch>
      </DashboardPage>);
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({isLoaded: true});
      });
  }

  private renderAccountPage = () => {
    const model = (() => {
      const pattern = Path.pathToRegexp(
        '/account/:id(\\d+)?', [], { end: false });
      const match = pattern.exec(window.location.pathname);
      const account = (() => {
        if(match[1]) {
          return Beam.DirectoryEntry.makeAccount(parseInt(match[1]), '');
        }
        return this.props.model.account;
      })();
      return this.props.model.makeAccountModel(account);
    })();
    return (
      <AccountController
        entitlements={this.props.model.entitlementDatabase}
        countryDatabase={this.props.model.countryDatabase}
        currencyDatabase={this.props.model.currencyDatabase}
        marketDatabase={this.props.model.marketDatabase}
        model={model}
        displaySize={this.props.displaySize}/>);
  }

  private onSideMenuClick(item: SideMenu.Item) {
    if(item === SideMenu.Item.SIGN_OUT) {
      this.props.model.logout().then(this.props.onLogout);
    }
  }
}
