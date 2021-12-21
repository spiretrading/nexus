import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { CreateAccountModel, GroupSuggestionModel, HttpCreateAccountModel,
  LocalGroupSuggestionModel } from '../create_account_page';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel using HTTP requests. */
export class HttpAccountDirectoryModel extends AccountDirectoryModel {

  /**
   * Constructs an HttpAccountDirectoryModel.
   * @param account The account whose trading groups are modelled.
   * @param serviceClients - The ServiceClients used to query.
   */
  constructor(
      account: Beam.DirectoryEntry, serviceClients: Nexus.ServiceClients) {
    super();
    this.account = account;
    this.serviceClients = serviceClients;
    this._organizationGroup = null;
    this._createAccountModel = new HttpCreateAccountModel(serviceClients);
    this._groupSuggestionModel = new LocalGroupSuggestionModel([]);
  }

  public get groups(): Beam.DirectoryEntry[] {
    return this._groups.slice();
  }

  public get organizationGroup(): Beam.DirectoryEntry {
    return this._organizationGroup;
  }

  public get createAccountModel(): CreateAccountModel {
    return this._createAccountModel;
  }

  public get groupSuggestionModel(): GroupSuggestionModel {
    return this._groupSuggestionModel;
  } 

  public async createGroup(name: string): Promise<Beam.DirectoryEntry> {
    const group =
      await this.serviceClients.administrationClient.createGroup(name);
    this._groups.push(group);
    this._groupSuggestionModel.addGroup(group);
    return group;
  }

  public async loadAccounts(group: Beam.DirectoryEntry):
      Promise<AccountEntry[]> {
    if(this.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR) &&
        group.id === this.tradingGroupsRoot.id) {
      const roles = new Nexus.AccountRoles();
      roles.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
      roles.set(Nexus.AccountRoles.Role.SERVICE);
      const accounts = await
        this.serviceClients.administrationClient.loadAccountsByRoles(roles);
      const accountEntries = [];
      for(const account of accounts) {
        accountEntries.push(new AccountEntry(account,
          await this.serviceClients.administrationClient.loadAccountRoles(
            account)));
      }
      return accountEntries;
    }
    const tradingGroup =
      await this.serviceClients.administrationClient.loadTradingGroup(group);
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
    return accounts.filter(
      (value: AccountEntry, index: number, accounts: AccountEntry[]) => {
        return accounts.findIndex((target: AccountEntry) =>
          (target.account.id === value.account.id)) === index});
  }

  public async loadFilteredAccounts(filter: string):
      Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    const matches =
      await this.serviceClients.administrationClient.searchAccounts(filter);
    const result = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    for(const match of matches) {
      let entries = result.get(match[0]);
      if(entries === undefined) {
        entries = [];
        result.set(match[0], entries);
      }
      if(match[1].type === Beam.DirectoryEntry.Type.ACCOUNT &&
          entries.findIndex((entry: AccountEntry) =>
            (entry.account.id === match[1].id)) < 0) {
        entries.push(new AccountEntry(match[1], match[2]));
      }
    }
    return result;
  }

  public async load(): Promise<void> {
    this.roles =
      await this.serviceClients.administrationClient.loadAccountRoles(
        await this.serviceClients.serviceLocatorClient.loadCurrentAccount());
    this._groups =
      await this.serviceClients.administrationClient.loadManagedTradingGroups(
        this.account);
    if(this.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
      this.tradingGroupsRoot = await
        this.serviceClients.administrationClient.loadTradingGroupsRootEntry();
      this._organizationGroup = Beam.DirectoryEntry.makeDirectory(
        this.tradingGroupsRoot.id,
        this.serviceClients.definitionsClient.organizationName);
      this._groups.push(this._organizationGroup);
    }
    this._groupSuggestionModel = new LocalGroupSuggestionModel(this._groups);
  }

  private account: Beam.DirectoryEntry;
  private roles: Nexus.AccountRoles;
  private tradingGroupsRoot: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private _groups: Beam.DirectoryEntry[];
  private _organizationGroup: Beam.DirectoryEntry;
  private _createAccountModel: HttpCreateAccountModel;
  private _groupSuggestionModel: LocalGroupSuggestionModel;
}
