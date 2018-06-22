import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as Router from 'react-router-dom';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

/** The properties used to control the application. */
interface Properties {}

interface State {
  account: Beam.DirectoryEntry;
  isLoading: boolean;
}

class AuthenticatedRoute extends React.Component<any> {
  public render(): JSX.Element {
    return (
      <Router.Route
        {...this.props}
        render={(props) => {
          if(this.props.account.equals(Beam.DirectoryEntry.INVALID)) {
            return <Router.Redirect to='/login'/>;
          } else {
            return this.props.render();
          }
        }}/>);
  }
}

/** The main entry point to the React application. */
class Application extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      account: Beam.DirectoryEntry.INVALID,
      isLoading: true
    };
    this.serviceClients = new Nexus.WebServiceClients();
    this.onLogin = this.onLogin.bind(this);
    this.onSignOut = this.onSignOut.bind(this);
  }

  public render(): JSX.Element {
    if(this.state.isLoading) {
      return <div></div>;
    }
    return (
      <Router.BrowserRouter>
        <Router.Switch>
          <Router.Route exact path='/login'
            render={() => {
              if(this.state.account === Beam.DirectoryEntry.INVALID) {
                return <WebPortal.LoginPage model={this.loginPageModel}
                  onLogin={this.onLogin}/>;
              }
              return <Router.Redirect to='/'/>;
            }}/>
          <AuthenticatedRoute path='/' account={this.state.account}
            render={() => {
              return <WebPortal.DashboardPage/>;
            }}/>
        </Router.Switch>
      </Router.BrowserRouter>);
  }

  public componentDidMount() {
    this.serviceClients.serviceLocatorClient.loadCurrentAccount().then(
      (account: Beam.DirectoryEntry) => {
        if(account.equals(Beam.DirectoryEntry.INVALID)) {
          this.loginPageModel = new WebPortal.WebLoginPageModel(
            this.serviceClients);
          this.setState({
            isLoading: false
          });
        } else {
          this.serviceClients.open().then(
            () => {
              this.setState({
                account: account,
                isLoading: false
              });
          });
        }
      });
  }

  private onLogin(account: Beam.DirectoryEntry): void {
    this.setState({
      account: account
    });
  }

  private onSignOut(): void {
    this.setState({
      account: Beam.DirectoryEntry.INVALID
    });
  }

  private serviceClients: Nexus.WebServiceClients;
  private loginPageModel: WebPortal.LoginPageModel;
}

ReactDOM.render(<Application/>, document.getElementById('main'));
