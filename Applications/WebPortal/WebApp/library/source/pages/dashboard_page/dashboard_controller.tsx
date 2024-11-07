import * as Beam from 'beam';
import * as React from 'react';
import * as Router from 'react-router-dom';
import * as Path from 'path-to-regexp';
import { DisplaySize, LoadingPage, PageNotFoundPage } from '../..';
import { AccountController, AccountDirectoryController, CreateAccountController,
  GroupController } from '..';
import { DashboardModel } from './dashboard_model';
import { DashboardPage } from './dashboard_page';
import { SideMenu } from './side_menu';

interface Properties extends Router.RouteComponentProps {

  /** The model to use. */
  model: DashboardModel;

  /** The device's display size. */
  displaySize: DisplaySize;

  /** Indicates the user has logged out. */
  onLogout?: () => void;
}

interface State {
  isLoaded: boolean;
  cannotLoad: boolean;
  redirect: string;
}

/** Implements the controller for the DashboardPage. */
export class DashboardController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      cannotLoad: false,
      redirect: null
    };
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(this.state.cannotLoad) {
      return <PageNotFoundPage displaySize={this.props.displaySize}/>;
    }
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
          <Router.Route path='/group' render={this.renderGroupPage}/>
          <Router.Route render={this.renderPageNotFound}/>
        </Router.Switch>
      </DashboardPage>);
  }

  public async componentDidMount(): Promise<void> {
    try {
      await this.props.model.load();
      if(this.props.location.pathname === '/') {
        this.setState({isLoaded: true, redirect: '/account'});
      } else {
        this.setState({isLoaded: true});
      }
    } catch {
      this.setState({cannotLoad: true});
    }
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private renderAccountPage = (props: Router.RouteComponentProps) => {
    const model = (() => {
      const match = DashboardController.ACCOUNT_PATTERN.regexp.exec(
        this.props.location.pathname);
      const account = (() => {
        if(match?.[1]) {
          return Beam.DirectoryEntry.makeAccount(parseInt(match[1]), '');
        }
        return this.props.model.account;
      })();
      return this.props.model.makeAccountModel(account);
    })();
    return (
      <AccountController {...props}
        entitlements={this.props.model.entitlementDatabase}
        countryDatabase={this.props.model.countryDatabase}
        currencyDatabase={this.props.model.currencyDatabase}
        marketDatabase={this.props.model.marketDatabase}
        authenticatedAccount={this.props.model.account}
        roles={this.props.model.roles}
        model={model}
        displaySize={this.props.displaySize}/>);
  }

  private renderGroupPage = (props: Router.RouteComponentProps) => {
    const match = DashboardController.GROUP_PATTERN.regexp.exec(
      this.props.location.pathname);
    if(!match[1]) {
      return this.renderPageNotFound();
    }
    const group = Beam.DirectoryEntry.makeDirectory(parseInt(match[1]), '');
    const model = this.props.model.makeGroupModel(group);
    return <GroupController {...props} roles={this.props.model.roles}
      model={model} displaySize={this.props.displaySize}/>;
  }
  
  private renderPageNotFound = () => {
    return <PageNotFoundPage displaySize={this.props.displaySize}/>;
  }

  private onSideMenuClick = (item: SideMenu.Item) => {
    if(item === SideMenu.Item.SIGN_OUT) {
      this.props.model.logout().then(this.props.onLogout);
    }
  }

  private static readonly ACCOUNT_PATTERN = Path.pathToRegexp(
    '/account{/:id}', { end: false });
  private static readonly GROUP_PATTERN = Path.pathToRegexp(
    '/group{/:id}', { end: false });
}
