import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { CreateAccountModel, GroupSuggestionModel, HttpCreateAccountModel,
  LocalGroupSuggestionModel } from '../create_account_page';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';
import { runInThisContext } from 'vm';

/** Implements an AccountDirectoryModel using HTTP requests. */
export class HttpAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs an HttpAccountDirectoryModel.
   * @param account The account whose trading groups are modelled.
   * @param serviceClients - The ServiceClients used to query.
   */
  constructor(account: Beam.DirectoryEntry,
      serviceClients: Nexus.ServiceClients) {
    super();
    this.account = account;
    this.serviceClients = serviceClients;
    this._createAccountModel = new HttpCreateAccountModel(serviceClients);
    this._groupSuggestionModel = new LocalGroupSuggestionModel([]);
  }

  public async load(): Promise<void> {
    const groups =
      await this.serviceClients.administrationClient.loadManagedTradingGroups(
      this.account);
    this._groups = [];
    for(const group of groups) {
      this._groups.push(group);
    }
    this._groups.sort(this.groupComparator);
    this._groupSuggestionModel = new LocalGroupSuggestionModel(groups);
  }

  public get groups(): Beam.DirectoryEntry[] {
    return this._groups.slice();
  }

  public get createAccountModel(): CreateAccountModel {
    return this._createAccountModel;
  }

  public get groupSuggestionModel(): GroupSuggestionModel {
    return this._groupSuggestionModel;
  } 

  public async createGroup(name: string): Promise<Beam.DirectoryEntry> {
    const group = await this.serviceClients.administrationClient.createGroup(
      name);
    this._groups.push(group);
    this._groups.sort(this.groupComparator);
    return group;
  }

  public async loadAccounts(group: Beam.DirectoryEntry):
      Promise<AccountEntry[]> {
    const tradingGroup =
      await this.serviceClients.administrationClient.loadTradingGroup(group);
    const accounts = [] as AccountEntry[];
    for(const manager of tradingGroup.managers) {
      const roles =
        await this.serviceClients.administrationClient.loadAccountRoles(
        manager);
      accounts.push(new AccountEntry(manager, roles));
    }
    for(const trader of tradingGroup.traders) {
      const roles =
        await this.serviceClients.administrationClient.loadAccountRoles(trader);
      accounts.push(new AccountEntry(trader, roles));
    }
    accounts.sort(this.accountComparator);
    return accounts.filter(
      (value: AccountEntry, index: number, array: AccountEntry[]) => {
        return array.findIndex((target: AccountEntry) =>
          (target.account.id === value.account.id)) === index});
  }

  public async loadFilteredAccounts(
      filter: string): Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    const matches =
      await this.serviceClients.administrationClient.searchAccounts(filter);
    const result = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    for(const match of matches) {
      let entries = result.get(match[0]);
      if(entries === undefined) {
        entries = [];
        result.set(match[0], entries);
      }
      entries.sort(this.accountComparator);
      entries.push(new AccountEntry(match[1], match[2]));
    }
    return result;
  }

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private _groups: Beam.DirectoryEntry[];
  private _createAccountModel: HttpCreateAccountModel;
  private _groupSuggestionModel: LocalGroupSuggestionModel;
}
