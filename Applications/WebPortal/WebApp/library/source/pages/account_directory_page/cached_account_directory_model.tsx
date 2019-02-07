import * as Beam from 'beam';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel in memory. */
export class CachedAccountDirectoryModel extends AccountDirectoryModel {

  constructor(model: AccountDirectoryModel) {
    super();
    this._model = model;
    this._groups = new Beam.Set<Beam.DirectoryEntry>();
    this._accounts = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    this._prevFiltered = new
     Map<string,Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>();
  }

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    return this._model.groups;
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    if(!this._accounts.get(group)) {
      this._accounts.set(group, await this._model.loadAccounts(group));
    }
    return new Promise<AccountEntry[]>((resolve) => {
        setTimeout(() => {
          resolve(this._accounts.get(group));}, 100);
        });
  }

  public async loadFilteredAccounts(filter: string):
    Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
   let sub = filter.length;
   while(sub) {
    const substring = filter.substring(0, sub);
    if(this._prevFiltered.get(substring)) {
      if(filter !== substring) {
        const superset = this._prevFiltered.get(substring);
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
      return new Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>
        ((resolve) => {
          setTimeout(() => {
            resolve(this._prevFiltered.get(substring));}, 100);
          });
    }
    --sub;
   }
    const accounts = await this._model.loadFilteredAccounts(filter);
    this._prevFiltered.set(filter, accounts);
    return new Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>
      ((resolve) => {
        setTimeout(() => {
          resolve(this._prevFiltered.get(filter));}, 100);
    });
  }

  public load(): Promise<void> {
    return this._model.load();
  }

  private _groups: Beam.Set<Beam.DirectoryEntry>;
  private _model: AccountDirectoryModel;
  private _prevFiltered: Map<string,
    Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>;
  private _accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}
