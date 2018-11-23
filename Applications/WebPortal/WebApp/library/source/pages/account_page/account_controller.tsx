import * as React from 'react';
import * as Router from 'react-router-dom';
import { AccountModel } from './account_model';
import { AccountPage } from './account_page';
import { DisplaySize } from '../../display_size';
import { SubPage } from './sub_page';

interface Properties {

  /** The URL prefix to match. */
  urlPrefix?: string;

  /** Determines the layout to use based on the display device. */
  displaySize: DisplaySize;

  /** The model representing the account to display. */
  model: AccountModel;
}

interface State {
  isLoaded: boolean;
  redirect: string;
}

/** Implements a controller for the AccountPage. */
export class AccountController extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    urlPrefix: ''
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      redirect: null,
    };
    this.navigateToProfile = this.navigateToProfile.bind(this);
    this.navigateToEntitlements = this.navigateToEntitlements.bind(this);
    this.onMenuClick = this.onMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(!this.state.isLoaded) {
      return <div/>;
    }
    const subPage = (() => {
      if(window.location.href.endsWith('/profile')) {
        return SubPage.PROFILE;
      } else if(window.location.href.endsWith('/entitlements')) {
        return SubPage.ENTITLEMENTS;
      }
      return SubPage.NONE;
    })();
    if(subPage === SubPage.NONE) {
      return <Router.Redirect to={`${this.props.urlPrefix}/profile`}/>;
    }
    return (
      <Router.BrowserRouter>
        <Router.Switch>
          <AccountPage displaySize={this.props.displaySize}
              subPage={subPage} account={this.props.model.account}
              roles={this.props.model.roles} onMenuClick={this.onMenuClick}>
            <Router.Route path={`${this.props.urlPrefix}/profile`}
              render={this.navigateToProfile}/>
            <Router.Route path={`${this.props.urlPrefix}/entitlements`}
              render={this.navigateToEntitlements}/>
          </AccountPage>
        </Router.Switch>
      </Router.BrowserRouter>);
  }

  public componentWillMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({isLoaded: true});
      });
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private navigateToProfile() {
    return <div>Profile</div>;
  }

  private navigateToEntitlements() {
    return <div>Entitlements</div>;
  }

  private onMenuClick(subPage: SubPage) {
    if(subPage === SubPage.PROFILE) {
      this.setState({
        redirect: `${this.props.urlPrefix}/profile`,
      });
    } else if(subPage === SubPage.ENTITLEMENTS) {
      this.setState({
        redirect: `${this.props.urlPrefix}/entitlements`,
      });
    }
  }
}
