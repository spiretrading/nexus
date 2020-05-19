import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { CreateAccountModel } from './create_account_model';

/** Implements the CreateAccountModel using HTTP services. */
export class HttpCreateAccountModel extends CreateAccountModel {

  /** Constructs an HttpCreateAccountModel.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
  }

  public async createAccount(username: string, groups: Beam.DirectoryEntry,
    identity: Nexus.AccountIdentity, roles: Nexus.AccountRoles): Promise<void> {
    await this.serviceClients.administrationClient.createAccount(
      username, groups, identity, roles);
  }

  private serviceClients: Nexus.ServiceClients;
}