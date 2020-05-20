import * as Beam from 'beam';
import { CreateAccountModel, LocalCreateAccountModel, LocalGroupSuggestionModel,
  GroupSuggestionModel } from '..';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel that caches. */
export class CachedAccountDirectoryModel extends AccountDirectoryModel {
  
  /** Constructs an cached model from a existing model
   * @param model - The model to be used.
   */
  constructor(model: AccountDirectoryModel) {
    super();
    this._model = model;
    this._accounts = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    this._prevFiltered = new
      Map<string,Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>();
  }

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    return this._model.groups;
  }

  public get createAccountModel(): CreateAccountModel {
    return this._model.createAccountModel;
  }

  public get groupSuggestionModel(): GroupSuggestionModel {
    return this._model.groupSuggestionModel;
  }

  public async createGroup(name: string): Promise<Beam.DirectoryEntry> {
    const group = await this._model.createGroup(name);
    this._model.groups.add(group);
    return group;
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    if(!this._accounts.get(group)) {
      this._accounts.set(group, await this._model.loadAccounts(group));
    }
    return this._accounts.get(group);
  }

  public async loadFilteredAccounts(filter: string):
      Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    let substringLength = filter.length;
    while(substringLength) {
      const substring = filter.substring(0, substringLength);
      const superset = this._prevFiltered.get(substring);
      if(superset) {
        if(filter !== substring) {
          const subset = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
          for(const pair of superset) {
            const accountSubset: AccountEntry[] = [];
            for(const account of superset.get(pair[0])) {
              if(account.account.name.indexOf(filter) === 0) {
                accountSubset.push(account);
              }
            }
            subset.set(pair[0], accountSubset);
          }
          this._prevFiltered.set(filter, subset);
        }
        return this._prevFiltered.get(substring);
      }
      --substringLength;
    }
    const accounts = await this._model.loadFilteredAccounts(filter);
    this._prevFiltered.set(filter, accounts);
    return this._prevFiltered.get(filter);
  }

  public load(): Promise<void> {
    return this._model.load();
  }

  private _model: AccountDirectoryModel;
  private _prevFiltered: Map<string,
    Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>;
  private _accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}
