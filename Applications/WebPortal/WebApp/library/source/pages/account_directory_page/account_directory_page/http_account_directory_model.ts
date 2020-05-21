import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { CreateAccountModel, GroupSuggestionModel, HttpCreateAccountModel,
  LocalGroupSuggestionModel } from '../create_account_page';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

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
    this._groups = new Beam.Set<Beam.DirectoryEntry>();
    for(const group of groups) {
      this._groups.add(group);
    }
    this._groupSuggestionModel = new LocalGroupSuggestionModel(groups);
  }

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    return this._groups.clone();
  }

  public get createAccountModel(): CreateAccountModel {
    return this._createAccountModel;
  }

  public get groupSuggestionModel(): GroupSuggestionModel {
    return this._groupSuggestionModel;
  } 

  public async createGroup(name: string): Promise<Beam.DirectoryEntry> {
    return await this.serviceClients.administrationClient.createGroup(name);
  }

  public async loadAccounts(group: Beam.DirectoryEntry):
      Promise<AccountEntry[]> {
    const tradingGroup =
      await this.serviceClients.administrationClient.loadTradingGroup(group);
    const accounts = [];
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
    return accounts;
  }

  public async loadFilteredAccounts(
      filter: string): Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    const matches =
      await this.serviceClients.administrationClient.searchAccounts(filter);
    const result = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    for(const match of matches) {
      let entries = result.get(match[0]);
      if(entries === undefined) {
        entries = []
        result.set(match[0], entries);
      }
      entries.push(new AccountEntry(match[1], match[2]));
    }
    return result;
  }

  private account: Beam.DirectoryEntry;
  private serviceClients: Nexus.ServiceClients;
  private _groups: Beam.Set<Beam.DirectoryEntry>;
  private _createAccountModel: HttpCreateAccountModel;
  private _groupSuggestionModel: LocalGroupSuggestionModel;
}
