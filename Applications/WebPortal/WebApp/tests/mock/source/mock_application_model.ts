import * as Beam from 'beam';
import * as WebPortal from 'web_portal';

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
    return null;
  }

  public async loadAccount(): Promise<Beam.DirectoryEntry> {
    return this.account;
  }

  public reset(): void {
    this.account = Beam.DirectoryEntry.INVALID;
    this._loginModel = new WebPortal.LocalLoginModel();
  }

  private account: Beam.DirectoryEntry;
  private _loginModel: WebPortal.LoginModel;
}
