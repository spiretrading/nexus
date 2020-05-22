import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';
import { HttpEntitlementsModel } from './entitlements_page';
import { HttpProfileModel } from './profile_page';
import { HttpRiskModel } from './risk_page';
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
    this._entitlementsModel = new HttpEntitlementsModel(account,
      this.serviceClients);
    this._profileModel = new HttpProfileModel(account, this.serviceClients);
    this._riskModel = new HttpRiskModel(account, this.serviceClients);
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get roles(): Nexus.AccountRoles {
    return this.model.roles;
  }

  public get groups(): Beam.DirectoryEntry[] {
    return this.model.groups;
  }

  public get entitlementsModel(): HttpEntitlementsModel {
    return this._entitlementsModel;
  }

  public get profileModel(): HttpProfileModel {
    return this._profileModel;
  }

  public get riskModel(): HttpRiskModel {
    return this._riskModel;
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.model.load();
    const account = await (async () => {
      if(this.account.name) {
        return this.account;
      }
      return await
        this.serviceClients.serviceLocatorClient.loadDirectoryEntryFromId(
        this.account.id);
    })();
    const roles =
      await this.serviceClients.administrationClient.loadAccountRoles(account);
    this.model = new LocalAccountModel(account, roles);
    return this.model.load();
  }

  private model: LocalAccountModel;
  private serviceClients: Nexus.ServiceClients;
  private _entitlementsModel: HttpEntitlementsModel;
  private _profileModel: HttpProfileModel;
  private _riskModel: HttpRiskModel;
}
