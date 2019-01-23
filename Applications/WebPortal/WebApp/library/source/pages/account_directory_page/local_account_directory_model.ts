import * as Beam from 'beam';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel in memory. */
export class LocalAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs an new model.
   * @param groups - A set of groups.
   * @param accounts - A map of all the accounts associated with
   *        each group.
   */
  constructor(groups: Beam.Set<Beam.DirectoryEntry>,
      accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>) {
    super();
    this._isLoaded = false;
    this._groups = groups.clone();
    this._accounts = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    for(const thing in groups) {
    }

  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._groups.clone();
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._accounts.get(group).slice();
  }

  private _isLoaded: boolean;
  private _groups: Beam.Set<Beam.DirectoryEntry>;
  private _accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}
