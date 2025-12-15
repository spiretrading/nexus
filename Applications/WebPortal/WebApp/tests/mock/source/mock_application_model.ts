import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as WebPortal from 'web_portal';
import { MockLoginModel } from './mock_login_model';

/** Implements the ApplicationModel using mock data. */
export class MockApplicationModel extends WebPortal.ApplicationModel {
  constructor() {
    super();
    this.reset();
  }

  public get loginModel(): WebPortal.LoginModel {
    return this._loginModel;
  }

  public get dashboardModel(): WebPortal.DashboardModel {
    if(this._dashboardModel.account.equals(this._loginModel.account)) {
      return this._dashboardModel;
    }
    this._dashboardModel = new WebPortal.LocalDashboardModel(
      this._loginModel.account, new Nexus.AccountRoles(0),
      new Nexus.EntitlementDatabase(), Nexus.buildDefaultCountryDatabase(),
      Nexus.buildDefaultCurrencyDatabase(), Nexus.buildDefaultVenueDatabase(),
      new WebPortal.LocalAccountDirectoryModel(
        new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>()));
    this._dashboardModel.load();
    return this.dashboardModel;
  }

  public async loadAccount(): Promise<Beam.DirectoryEntry> {
    return this._loginModel.account;
  }

  public reset(): void {
    this._loginModel = new MockLoginModel();
    this._dashboardModel = new WebPortal.LocalDashboardModel(
      Beam.DirectoryEntry.INVALID, new Nexus.AccountRoles(0),
      new Nexus.EntitlementDatabase(), Nexus.buildDefaultCountryDatabase(),
      Nexus.buildDefaultCurrencyDatabase(), Nexus.buildDefaultVenueDatabase(),
      new WebPortal.LocalAccountDirectoryModel(
        new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>()));
    this._dashboardModel.load();
  }

  private _loginModel: WebPortal.LoginModel;
  private _dashboardModel: WebPortal.DashboardModel;
}
