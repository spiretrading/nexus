import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalProfileModel } from './local_profile_model';
import { ProfileModel } from './profile_model';

/** Implements the ProfileModel using HTTP services. */
export class HttpProfileModel extends ProfileModel {

  /** Constructs an HttpProfileModel.
   * @param account - The account to represent.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(account: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalProfileModel(account, new Nexus.AccountRoles(),
      new Nexus.AccountIdentity());
    this.serviceClients = serviceClients;
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get roles(): Nexus.AccountRoles {
    return this.model.roles;
  }

  public get groups(): Beam.DirectoryEntry[] {
    return this.model.groups.slice();
  }

  public get identity(): Nexus.AccountIdentity {
    return this.model.identity;
  }

  public async load(): Promise<void> {
    this.model.load();
    const roles =
      await this.serviceClients.administrationClient.loadAccountRoles(
      this.account);
    const identity =
      await this.serviceClients.administrationClient.loadAccountIdentity(
      this.account);
    const group = await
      this.serviceClients.administrationClient.loadParentTradingGroup(
        this.account);
    this.model.groups = [group];
    await this.model.updateIdentity(roles, identity);
  }

  public async updateIdentity(roles: Nexus.AccountRoles,
      identity: Nexus.AccountIdentity): Promise<void> {
    const updatedRoles =
      await this.serviceClients.administrationClient.storeAccountRoles(
      this.account, roles);
    await this.serviceClients.administrationClient.storeAccountIdentity(
      this.account, identity);
    await this.model.updateIdentity(updatedRoles, identity);
  }

  public async updatePassword(password: string): Promise<void> {
    await this.serviceClients.serviceLocatorClient.storePassword(this.account,
      password);
    await this.model.updatePassword(password);
  }

  private model: LocalProfileModel;
  private serviceClients: Nexus.ServiceClients;
}
