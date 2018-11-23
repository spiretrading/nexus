import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ApplicationModel } from './application_model'
import { WebDashboardModel } from './dashboard_page';
import { WebLoginModel } from './login_page';

/** Implements the ApplicationModel using web services. */
export class WebApplicationModel extends ApplicationModel {

  /** Constructs a WebApplicationModel from a set of ServiceClients.
   * @param serviceClients - The ServiceClients used to access the web services.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
  }

  public makeLoginModel(): WebLoginModel {
    return new WebLoginModel(this.serviceClients);
  }

  public makeDashboardModel(): WebDashboardModel {
    return new WebDashboardModel(this.serviceClients);
  }

  public async loadAccount(): Promise<Beam.DirectoryEntry> {
    return await this.serviceClients.serviceLocatorClient.loadCurrentAccount();
  }

  private serviceClients: Nexus.ServiceClients;
}
