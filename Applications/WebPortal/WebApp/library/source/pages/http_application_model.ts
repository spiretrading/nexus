import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ApplicationModel } from './application_model'
import { HttpDashboardModel } from './dashboard_page';
import { HttpLoginModel } from './login_page';

/** Implements the ApplicationModel using HTTP requests. */
export class HttpApplicationModel extends ApplicationModel {

  /** Constructs an HttpApplicationModel from a set of ServiceClients.
   * @param serviceClients - The ServiceClients used to access the HTTP
   *                         services.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
    this._loginModel = new HttpLoginModel(this.serviceClients);
    this._dashboardModel = new HttpDashboardModel(this.serviceClients);
  }

  public get loginModel(): HttpLoginModel {
    return this._loginModel;
  }

  public get dashboardModel(): HttpDashboardModel {
    return this._dashboardModel;
  }

  public async loadAccount(): Promise<Beam.DirectoryEntry> {
    return await this.serviceClients.serviceLocatorClient.loadCurrentAccount();
  }

  private serviceClients: Nexus.ServiceClients;
  private _loginModel: HttpLoginModel;
  private _dashboardModel: HttpDashboardModel;
}
