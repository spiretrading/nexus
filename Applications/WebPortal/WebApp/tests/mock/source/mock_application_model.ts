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
      new Nexus.EntitlementDatabase(), Nexus.buildCountryDatabase(),
      Nexus.buildCurrencyDatabase(), Nexus.buildVenueDatabase(),
      new WebPortal.LocalAccountDirectoryModel(
        new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>()),
      new WebPortal.LocalRequestsModel(
        this._loginModel.account, [], new Map()), makeReportModel());
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
      new Nexus.EntitlementDatabase(), Nexus.buildCountryDatabase(),
      Nexus.buildCurrencyDatabase(), Nexus.buildVenueDatabase(),
      new WebPortal.LocalAccountDirectoryModel(
        new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>()),
      new WebPortal.LocalRequestsModel(
        Beam.DirectoryEntry.INVALID, [], new Map()), makeReportModel());
    this._dashboardModel.load();
  }

  private _loginModel: WebPortal.LoginModel;
  private _dashboardModel: WebPortal.DashboardModel;
}

function makeReportModel(): WebPortal.ReportModel {
  const accounts = new WebPortal.LocalAccountGroupQueryModel([]);
  const scopes = new WebPortal.ScopeQueryModel(
    new WebPortal.LocalTickerQueryModel([]));
  return new WebPortal.CompositeReportModel(
    new WebPortal.LocalScheduledReportsModel([]),
    new WebPortal.LocalGeneratedReportsModel([], accounts),
    new WebPortal.LocalReportActivityModel([]),
    new WebPortal.LocalCreateReportModel([], accounts, scopes),
    new WebPortal.LocalEditScheduledReportModel(
      [], accounts, scopes, new Map()),
    new WebPortal.LocalReportDetailModel([]));
}
