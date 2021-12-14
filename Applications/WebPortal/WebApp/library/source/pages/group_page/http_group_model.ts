import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ComplianceModel, ComplianceService, HttpComplianceService } from '..';
import { AccountEntry } from '../account_directory_page';
import { GroupModel } from './group_model';
import { LocalGroupModel } from './local_group_model';

/** Implements the GroupModel using HTTP requests. */
export class HttpGroupModel extends GroupModel {

  /** Constructs an HttpGroupModel.
   * @param group - The group this model represents.
   * @param serviceClients - The clients used to access the HTTP services.
   */
  constructor(group: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.model = new LocalGroupModel(group, [], new ComplianceModel(group, [],
      [], serviceClients.definitionsClient.currencyDatabase));
    this.serviceClients = serviceClients;
    this._complianceService =
      new HttpComplianceService(group, this.serviceClients);
  }

  public get group(): Beam.DirectoryEntry {
    return this.model.group;
  }

  public get accounts(): AccountEntry[] {
    return this.model.accounts;
  }

  public get complianceService(): ComplianceService {
    return this._complianceService;
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.model.load();
    this.tradingGroup = await this.
      serviceClients.administrationClient.loadTradingGroup(this.model.group);
    const accounts = [] as AccountEntry[];
    for(const manager of this.tradingGroup.managers) {
      const roles = await this.
        serviceClients.administrationClient.loadAccountRoles(manager);
      accounts.push(new AccountEntry(manager, roles));
    }
    for(const trader of this.tradingGroup.traders) {
      const roles = await this.
        serviceClients.administrationClient.loadAccountRoles(trader);
      accounts.push(new AccountEntry(trader, roles));
    }
    const nonDuplicateAccounts = accounts.filter(
      (accountEntry: AccountEntry, index: number,
          accountEntries: AccountEntry[]) => {
        return accountEntries.findIndex((target: AccountEntry) =>
          (target.account.id === accountEntry.account.id)) === index});
    this.model = new LocalGroupModel(this.tradingGroup.entry,
      nonDuplicateAccounts, new ComplianceModel(this.tradingGroup.entry, [], [],
        this.serviceClients.definitionsClient.currencyDatabase));
    this._complianceService =
      new HttpComplianceService(this.tradingGroup.entry, this.serviceClients);
    await this.model.load();
  }

  private model: LocalGroupModel;
  private serviceClients: Nexus.ServiceClients;
  private tradingGroup: Nexus.TradingGroup;
  private _complianceService: HttpComplianceService;
}
