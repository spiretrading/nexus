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
  }

  public getLoginModel(): HttpLoginModel {
    return new HttpLoginModel(this.serviceClients);
  }

  public getDashboardModel(): HttpDashboardModel {
    return new HttpDashboardModel(this.serviceClients);
  }

  public async loadAccount(): Promise<Beam.DirectoryEntry> {
    return await this.serviceClients.serviceLocatorClient.loadCurrentAccount();
  }

  private serviceClients: Nexus.ServiceClients;
}
