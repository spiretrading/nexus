import * as Beam from 'beam';
import * as Router from 'react-router-dom';
import * as React from 'react';
import { DisplaySize } from '..';
import { ApplicationModel } from './application_model';
import { DashboardController, DashboardModel } from './dashboard_page';
import { LoginController, LoginModel } from './login_page';

interface Properties {

  /** The top-level application model. */
  model: ApplicationModel;
}

interface State {
  displaySize: DisplaySize;
  account: Beam.DirectoryEntry;
  isLoading: boolean;
}

/** The main entry point to the application. */
export class ApplicationController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      displaySize: DisplaySize.getDisplaySize(),
      account: Beam.DirectoryEntry.INVALID,
      isLoading: true
    };
    this.dashboardModel = null;
    this.onResize = this.onResize.bind(this);
    this.onLogin = this.onLogin.bind(this);
    this.onLogout = this.onLogout.bind(this);
    this.navigateToLogin = this.navigateToLogin.bind(this);
    this.navigateToDashboard = this.navigateToDashboard.bind(this);
  }

  public render(): JSX.Element {
    if(this.state.isLoading) {
      return <div></div>;
    }
    return (
      <Router.BrowserRouter>
        <Router.Switch>
          <Router.Route exact path='/login' render={this.navigateToLogin}/>
          <AuthenticatedRoute path='/' account={this.state.account}
            render={this.navigateToDashboard}/>
        </Router.Switch>
      </Router.BrowserRouter>);
  }

  public componentWillMount(): void {
    this.props.model.loadAccount().then(
      (account) => {
        this.setState({
          account: account,
          isLoading: false
        });
      });
  }

  public componentDidMount(): void {
    window.addEventListener('resize', this.onResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onResize);
  }

  private onResize() {
    const displaySize = DisplaySize.getDisplaySize();
    if(displaySize !== this.state.displaySize) {
      this.setState({displaySize: displaySize});
    }
  }

  private onLogin(account: Beam.DirectoryEntry) {
    this.setState({
      account: account
    });
  }

  private onLogout() {
    this.setState({
      account: Beam.DirectoryEntry.INVALID
    });
  }

  private navigateToLogin() {
    if(this.state.account.equals(Beam.DirectoryEntry.INVALID)) {
      if(this.loginModel == null) {
        this.loginModel = this.props.model.makeLoginModel();
      }
      return <LoginController model={this.loginModel} onLogin={this.onLogin}/>;
    }
    return <Router.Redirect to='/'/>;
  }

  private navigateToDashboard() {
    if(this.dashboardModel === null) {
      this.dashboardModel = this.props.model.makeDashboardModel();
    }
    return <DashboardController model={this.dashboardModel}
      displaySize={this.state.displaySize} onLogout={this.onLogout}/>;
  }

  private loginModel: LoginModel;
  private dashboardModel: DashboardModel;
}

class AuthenticatedRoute extends React.Component<any> {
  public render(): JSX.Element {
    return (
      <Router.Route
        {...this.props}
        render={(_) => {
          if(this.props.account.equals(Beam.DirectoryEntry.INVALID)) {
            return <Router.Redirect to='/login'/>;
          } else {
            return this.props.render();
          }
        }}/>);
  }
}
