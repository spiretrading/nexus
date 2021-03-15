import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { DisplaySize, LoadingPage, PageNotFoundPage } from '../..';
import { AccountModel } from './account_model';
import { AccountPage } from './account_page';
import { EntitlementsController } from './entitlements_page';
import { ProfileController } from './profile_page';
import { RiskController } from './risk_page';
import { SubPage } from './sub_page';

interface Properties {

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The database of currencies. */
  countryDatabase: Nexus.CountryDatabase;

  /** The database of countries. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The set of markets. */
  marketDatabase: Nexus.MarketDatabase;

  /** Determines the layout to use based on the display device. */
  displaySize: DisplaySize;

  /** The authenticated user's account. */
  authenticatedAccount: Beam.DirectoryEntry;
  
  /** The authenticated user's roles. */
  roles: Nexus.AccountRoles;

  /** The model representing the account to display. */
  model: AccountModel;
}

interface State {
  isLoaded: boolean;
  cannotLoad: boolean;
  redirect: string;
  readonly: boolean;
  isPasswordReadOnly: boolean;
}

/** Implements a controller for the AccountPage. */
export class AccountController extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      cannotLoad: false,
      redirect: null,
      readonly: true,
      isPasswordReadOnly: true
    };
    this.renderProfilePage = this.renderProfilePage.bind(this);
    this.renderEntitlementsPage = this.renderEntitlementsPage.bind(this);
    this.renderRiskPage = this.renderRiskPage.bind(this);
    this.onMenuClick = this.onMenuClick.bind(this);
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
    const subPage = (() => {
      if(window.location.pathname.endsWith('/profile')) {
        return SubPage.PROFILE;
      } else if(window.location.pathname.endsWith('/entitlements')) {
        return SubPage.ENTITLEMENTS;
      } else if(window.location.pathname.endsWith('/risk')) {
        return SubPage.RISK_CONTROLS;
      }
      return SubPage.NONE;
    })();
    return (
      <AccountPage displaySize={this.props.displaySize}
          subPage={subPage} account={this.props.model.account}
          roles={this.props.model.roles} onMenuClick={this.onMenuClick}>
        <Router.Switch>
          <Router.Route path='/account/:id(\d+)?/profile'
            render={this.renderProfilePage}/>
          <Router.Route path='/account/:id(\d+)?/entitlements'
            render={this.renderEntitlementsPage}/>
          <Router.Route path='/account/:id(\d+)?/risk'
            render={this.renderRiskPage}/>
          <Router.Route path='/account/:id(\d+)?'
            render={({match}: any) => {
              const url = (() => {
                if(match.params.id) {
                  return `/account/${match.params.id}/profile`;
                }
                return '/account/profile';
              })();
              return <Router.Redirect to={url}/>;
            }}/>
        </Router.Switch>
      </AccountPage>);
  }

  public async componentDidMount(): Promise<void> {
    try {
      await this.props.model.load();
      const readonly = 
        !(this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR) && (
          this.props.authenticatedAccount.equals(this.props.model.account) ||
          this.props.model.roles.test(Nexus.AccountRoles.Role.TRADER) ||
          this.props.model.roles.test(Nexus.AccountRoles.Role.MANAGER)));
      const isPasswordReadOnly =
        !(this.props.authenticatedAccount.equals(this.props.model.account) ||
        !readonly);
      this.setState({
        isLoaded: true,
        readonly: readonly,
        isPasswordReadOnly: isPasswordReadOnly
      });
    } catch {
      this.setState({cannotLoad: true});
    }
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private parseUrlPrefix(): string {
    const url = window.location.pathname;
    const prefix = url.substr(0, url.lastIndexOf('/'));
    if(prefix === '') {
      return url;
    }
    return prefix;
  }

  private renderProfilePage() {
    return <ProfileController
      displaySize={this.props.displaySize}
      countryDatabase={this.props.countryDatabase}
      groups={this.props.model.groups}
      readonly={this.state.readonly}
      isPasswordReadOnly={this.state.isPasswordReadOnly}
      model={this.props.model.profileModel}/>;
  }

  private renderEntitlementsPage() {
    return <EntitlementsController roles={this.props.model.roles}
      entitlements={this.props.entitlements}
      model={this.props.model.entitlementsModel}
      currencyDatabase={this.props.currencyDatabase}
      marketDatabase={this.props.marketDatabase}
      displaySize={this.props.displaySize}/>;
  }

  private renderRiskPage() {
    return <RiskController
      currencyDatabase={this.props.currencyDatabase}
      displaySize={this.props.displaySize}
      model={this.props.model.riskModel}
      roles={this.props.model.roles}/>;
  }

  private onMenuClick(subPage: SubPage) {
    const urlPrefix = this.parseUrlPrefix();
    if(subPage === SubPage.PROFILE) {
      this.setState({redirect: `${urlPrefix}/profile`});
    } else if(subPage === SubPage.ENTITLEMENTS) {
      this.setState({redirect: `${urlPrefix}/entitlements`});
    } else if(subPage === SubPage.RISK_CONTROLS) {
      this.setState({redirect: `${urlPrefix}/risk`});
    }
  }
}
