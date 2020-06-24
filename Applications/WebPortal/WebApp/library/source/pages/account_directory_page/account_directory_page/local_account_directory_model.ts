import * as Beam from 'beam';
import { CreateAccountModel, GroupSuggestionModel, LocalCreateAccountModel,
  LocalGroupSuggestionModel } from '../..'
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel in memory. */
export class LocalAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs a new model.
   * @param accounts - A map of all the accounts associated with
   *        each group.
   */
  constructor(accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>) {
    super();
    this._isLoaded = false;
    this.nextId = 1;
    this._groups = [] as Beam.DirectoryEntry[];
    for(const account of accounts) {
      this._groups.push(account[0]);
    }
    this._accounts = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    for(const group of this._groups) {
      this.nextId = Math.max(this.nextId, group.id + 1);
      this._accounts.set(group, accounts.get(group).slice());
    }
    this._createAccountModel = new LocalCreateAccountModel();
    const groupsArray = [];
    for(const group of this._groups) {
      groupsArray.push(group);
    }
    this._groupAccountModel = new LocalGroupSuggestionModel(groupsArray);
  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public get groups(): Beam.DirectoryEntry[] {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._groups.slice();
  }

  public get createAccountModel(): CreateAccountModel{
    return this._createAccountModel;
  }

  public get groupSuggestionModel(): GroupSuggestionModel {
    return this._groupAccountModel;
  }

  public async createGroup(name: string): Promise<Beam.DirectoryEntry> {
    for(const group of this._groups) {
      if(group.name === name) {
        throw new Beam.ServiceError('Group already exists.');
      }
    }
    const group = Beam.DirectoryEntry.makeAccount(this.nextId, name);
    this._groups.push(group);
    ++this.nextId;
    return group;
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._accounts.get(group).slice();
  }

  public async loadFilteredAccounts(
      filter: string): Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    const matches = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    if(filter) {
      for(const group of this._groups) {
        const accounts: AccountEntry[] = [];
        for(const account of this._accounts.get(group)) {
          if(account.account.name.indexOf(filter) === 0) {
            accounts.push(account);
          }
        }
        if(accounts) {
          matches.set(group, accounts);
        }
      }
    }
    return matches;
  }

  private _isLoaded: boolean;
  private nextId: number;
  private _groups: Beam.DirectoryEntry[];
  private _accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
  private _createAccountModel: CreateAccountModel;
  private _groupAccountModel: GroupSuggestionModel;
}
