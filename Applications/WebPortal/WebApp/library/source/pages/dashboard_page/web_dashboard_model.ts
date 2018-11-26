import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { WebAccountModel } from '../account_page';
import { DashboardModel, LocalDashboardModel } from '.';

/** Implements the DashboardModel using web services. */
export class WebDashboardModel extends DashboardModel {

  /** Constructs a WebDashboardModel.
   * @param serviceClients - The service clients used to access the web
   *        services.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
    this.model = new LocalDashboardModel(Beam.DirectoryEntry.INVALID,
      new Nexus.AccountRoles(0));
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get roles(): Nexus.AccountRoles {
    return this.model.roles;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): WebAccountModel {
    return new WebAccountModel(account, this.serviceClients);
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    const account = await
      this.serviceClients.serviceLocatorClient.loadCurrentAccount();
    const roles = await
      this.serviceClients.administrationClient.loadAccountRoles(account);
    this.model = new LocalDashboardModel(account, roles);
    return this.model.load();
  }

  public async logout(): Promise<void> {
    this.serviceClients.close();
  }

  private serviceClients: Nexus.ServiceClients;
  private model: LocalDashboardModel;
}
