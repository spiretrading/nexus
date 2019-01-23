import * as Beam from 'beam';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an EntitlementsModel in memory. */
export class LocalAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs an new model.
   * @param groupList - The account to represent.
   * @param accountLists - The set of entitlements granted to the account.
   */
  constructor( groupList: Beam.Set<Beam.DirectoryEntry>,
      accountLists: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>) {
    super();
    this._isLoaded = false;
    this._groupList = groupList;
    this._accountLists = accountLists;
  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._isLoaded;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    return this._groupList;
  }

  public async loadAccounts(group: Beam.DirectoryEntry
    ): Promise<AccountEntry[]> {
      return this._accountLists.get(group);
    }

  private _isLoaded: boolean;
  private _groupList: Beam.Set<Beam.DirectoryEntry>;
  private _accountLists: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}
