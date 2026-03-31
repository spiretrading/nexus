import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';
import { ComplianceModel, ComplianceService, HttpComplianceService } from
  './compliance_page';
import { HttpEntitlementsModel } from './entitlements_page';
import { HttpProfileModel } from './profile_page';
import { HttpProfitAndLossModel } from './profit_and_loss_page';
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
    const roles = new Nexus.AccountRoles(0);
    this.model = new LocalAccountModel(account, roles, [],
      new ComplianceModel(account, [], [], new Nexus.CurrencyDatabase()));
    this._currency = Nexus.Currency.NONE;
    this.serviceClients = serviceClients;
    this._entitlementsModel =
      new HttpEntitlementsModel(account, this.serviceClients);
    this._profileModel = new HttpProfileModel(account, this.serviceClients);
    this._profitAndLossModel =
      new HttpProfitAndLossModel(account, this.serviceClients);
    this._riskModel = new HttpRiskModel(account, this.serviceClients);
    this._complianceService =
      new HttpComplianceService(account, this.serviceClients);
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

  public get currency(): Nexus.Currency {
    return this._currency;
  }

  public get entitlementsModel(): HttpEntitlementsModel {
    return this._entitlementsModel;
  }

  public get profileModel(): HttpProfileModel {
    return this._profileModel;
  }

  public get profitAndLossModel(): HttpProfitAndLossModel {
    return this._profitAndLossModel;
  }

  public get riskModel(): HttpRiskModel {
    return this._riskModel;
  }

  public get complianceService(): ComplianceService {
    return this._complianceService;
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
    const riskParameters =
      await this.serviceClients.administrationClient.loadRiskParameters(
        account);
    this._currency = riskParameters.currency;
    const complianceRuleEntries =
      await this.serviceClients.complianceClient.load(account);
    this.model =
      new LocalAccountModel(account, roles, groups, new ComplianceModel(
        this.account,
        this.serviceClients.definitionsClient.complianceRuleSchemas,
        complianceRuleEntries,
        this.serviceClients.definitionsClient.currencyDatabase));
    this._entitlementsModel =
      new HttpEntitlementsModel(account, this.serviceClients);
    this._profileModel = new HttpProfileModel(account, this.serviceClients);
    this._profitAndLossModel =
      new HttpProfitAndLossModel(account, this.serviceClients);
    this._riskModel = new HttpRiskModel(account, this.serviceClients);
    this._complianceService =
      new HttpComplianceService(account, this.serviceClients);
    return this.model.load();
  }

  private _currency: Nexus.Currency;
  private model: LocalAccountModel;
  private serviceClients: Nexus.ServiceClients;
  private _entitlementsModel: HttpEntitlementsModel;
  private _profileModel: HttpProfileModel;
  private _profitAndLossModel: HttpProfitAndLossModel;
  private _riskModel: HttpRiskModel;
  private _complianceService: HttpComplianceService;
}
