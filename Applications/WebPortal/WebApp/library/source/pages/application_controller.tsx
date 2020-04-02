import * as Beam from 'beam';
import * as Router from 'react-router-dom';
import * as React from 'react';
import { DisplaySize } from '..';
import { ApplicationModel } from './application_model';
import { DashboardController } from './dashboard_page';
import { LoadingPage } from './loading_page';
import { LoginController } from './login_page';

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
    this.onResize = this.onResize.bind(this);
    this.onLogin = this.onLogin.bind(this);
    this.onLogout = this.onLogout.bind(this);
    this.renderLoginPage = this.renderLoginPage.bind(this);
    this.renderDashboardPage = this.renderDashboardPage.bind(this);
  }

  public render(): JSX.Element {
    if(this.state.isLoading) {
      return <LoadingPage/>;
    }
    return (
      <Router.BrowserRouter>
        <Router.Switch>
          <Router.Route exact path='/login' render={this.renderLoginPage}/>
          <AuthenticatedRoute path='/' account={this.state.account}
            render={this.renderDashboardPage}/>
        </Router.Switch>
      </Router.BrowserRouter>);
  }

  public componentDidMount(): void {
    window.addEventListener('resize', this.onResize);
    this.props.model.loadAccount().then(
      (account) => {
        this.setState({
          account: account,
          isLoading: false
        });
      });
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

  private renderLoginPage() {
    if(this.state.account.equals(Beam.DirectoryEntry.INVALID)) {
      return <LoginController model={this.props.model.loginModel}
        onLogin={this.onLogin}/>;
    }
    return <Router.Redirect to='/'/>;
  }

  private renderDashboardPage() {
    return <DashboardController model={this.props.model.dashboardModel}
      displaySize={this.state.displaySize} onLogout={this.onLogout}/>;
  }
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
