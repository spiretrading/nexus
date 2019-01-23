import * as Beam from 'beam';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel in menory. */
export class LocalAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs an new model.
   * @param groups - A set of groups.
   * @param accounts  - A map of all the accounts associated with
   *  each group.
   */
  constructor(groups: Beam.Set<Beam.DirectoryEntry>,
      accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>) {
    super();
    this._isLoaded = false;
    this._groupList = groups.clone();
    this._accountLists = accounts;
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
    return this._groupList.clone();
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    return this._accountLists.get(group).slice();
  }

  private _isLoaded: boolean;
  private _groupList: Beam.Set<Beam.DirectoryEntry>;
  private _accountLists: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}
