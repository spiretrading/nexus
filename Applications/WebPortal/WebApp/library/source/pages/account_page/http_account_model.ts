import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';
import { HttpEntitlementsModel } from './entitlements_page';
import { LocalAccountModel } from './local_account_model';

/** Implements an AccountModel using HTTP services. */
export class HttpAccountModel extends AccountModel {

  /** Constructs an HttpAccountModel.
   * @param account - The account this model represents.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(account: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalAccountModel(account, new Nexus.AccountRoles(0));
    this.serviceClients = serviceClients;
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get roles(): Nexus.AccountRoles {
    return this.model.roles;
  }

  public makeEntitlementsModel(): HttpEntitlementsModel {
    return new HttpEntitlementsModel(this.account, this.serviceClients);
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.model.load();
    const roles =
      await this.serviceClients.administrationClient.loadAccountRoles(
      this.account);
    this.model = new LocalAccountModel(this.account, roles);
    return this.model.load();
  }

  private model: LocalAccountModel;
  private serviceClients: Nexus.ServiceClients;
}
