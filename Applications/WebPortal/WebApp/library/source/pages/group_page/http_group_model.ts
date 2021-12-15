import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ComplianceModel, ComplianceService, HttpComplianceService } from '..';
import { AccountEntry } from '../account_directory_page';
import { GroupModel } from './group_model';
import { LocalGroupModel } from './local_group_model';

/** Implements the GroupModel using HTTP requests. */
export class HttpGroupModel extends GroupModel {

  /**
   * Constructs an HttpGroupModel.
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
    const roles =
      await this.serviceClients.administrationClient.loadAccountRoles(
        await this.serviceClients.serviceLocatorClient.loadCurrentAccount());
    await this.model.load();
    if(roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
      const tradingGroupsRoot = await
        this.serviceClients.administrationClient.loadTradingGroupsRootEntry();
      if(this.model.group.id === tradingGroupsRoot.id) {
        return this.loadOrganizationGroup(tradingGroupsRoot);
      }
    }
    return this.loadTradingGroup();
  }

  private async loadOrganizationGroup(tradingGroupsRoot: Beam.DirectoryEntry) {
    const roles = new Nexus.AccountRoles();
    roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    roles.set(Nexus.AccountRoles.Role.SERVICE);
    const accounts =
      await this.serviceClients.administrationClient.loadAccountsByRoles(roles);
    const accountEntries = [];
    for(const account of accounts) {
      accountEntries.push(new AccountEntry(account,
        await this.serviceClients.administrationClient.loadAccountRoles(
          account)));
    }
    this.model = new LocalGroupModel(this.model.group, accountEntries,
      new ComplianceModel(this.model.group, [], [],
        this.serviceClients.definitionsClient.currencyDatabase));
    this._complianceService =
      new HttpComplianceService(tradingGroupsRoot, this.serviceClients);
    await this.model.load();
  }

  private async loadTradingGroup() {
    const tradingGroup = await this.
      serviceClients.administrationClient.loadTradingGroup(this.model.group);
    const accounts = [] as AccountEntry[];
    for(const manager of tradingGroup.managers) {
      const roles = await
        this.serviceClients.administrationClient.loadAccountRoles(manager);
      accounts.push(new AccountEntry(manager, roles));
    }
    for(const trader of tradingGroup.traders) {
      const roles = await
        this.serviceClients.administrationClient.loadAccountRoles(trader);
      accounts.push(new AccountEntry(trader, roles));
    }
    const nonDuplicateAccounts = accounts.filter(
      (entry: AccountEntry, index: number, entries: AccountEntry[]) => {
        return entries.findIndex((target: AccountEntry) =>
          (target.account.id === entry.account.id)) === index});
    this.model = new LocalGroupModel(tradingGroup.entry, nonDuplicateAccounts,
      new ComplianceModel(tradingGroup.entry, [], [],
        this.serviceClients.definitionsClient.currencyDatabase));
    this._complianceService =
      new HttpComplianceService(tradingGroup.entry, this.serviceClients);
    await this.model.load();
  }

  private model: LocalGroupModel;
  private serviceClients: Nexus.ServiceClients;
  private _complianceService: HttpComplianceService;
}
