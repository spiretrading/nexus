import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';
import { ComplianceModel, ComplianceService, LocalComplianceService } from
  './compliance_page';
import { HttpEntitlementsModel } from './entitlements_page';
import { HttpProfileModel } from './profile_page';
import { HttpRiskModel } from './risk_page';
import { LocalAccountModel } from './local_account_model';

/** Implements an AccountModel using HTTP services. */
export class HttpAccountModel extends AccountModel {

  /**
   * Constructs an HttpAccountModel.
   * @param account - The account this model represents.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(
      account: Beam.DirectoryEntry, serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalAccountModel(account, new Nexus.AccountRoles(0), [],
      new ComplianceModel(account, new Nexus.AccountRoles(0), [], [],
        new Nexus.CurrencyDatabase()));
    this.serviceClients = serviceClients;
    this._entitlementsModel =
      new HttpEntitlementsModel(account, this.serviceClients);
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

  public get complianceService(): ComplianceService {
    return this.model.complianceService;
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
    const groups = await (async () => {
      const group =
        await this.serviceClients.administrationClient.loadParentTradingGroup(
          account);
      if(group.equals(Beam.DirectoryEntry.INVALID)) {
        return [];
      }
      return [group];
    })();
    const complianceRuleEntries =
      await this.serviceClients.complianceClient.load(account);
    this.model =
      new LocalAccountModel(account, roles, groups, new ComplianceModel(
        this.account, roles,
        this.serviceClients.definitionsClient.complianceRuleSchemas,
        complianceRuleEntries,
        this.serviceClients.definitionsClient.currencyDatabase));
    this._entitlementsModel =
      new HttpEntitlementsModel(account, this.serviceClients);
    this._profileModel = new HttpProfileModel(account, this.serviceClients);
    this._riskModel = new HttpRiskModel(account, this.serviceClients);
    return this.model.load();
  }

  private model: LocalAccountModel;
  private serviceClients: Nexus.ServiceClients;
  private _entitlementsModel: HttpEntitlementsModel;
  private _profileModel: HttpProfileModel;
  private _riskModel: HttpRiskModel;
}
