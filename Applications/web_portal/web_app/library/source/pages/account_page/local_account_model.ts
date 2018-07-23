import * as Beam from 'beam';
import * as Nexus from 'nexus';
import {AccountModel} from './account_model';

/** Implements an AccountModel locally. */
export class LocalAccountModel extends AccountModel {

  /** Constructs a LocalAccountModel. */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles) {
    super();
    this.is_loaded = false;
    this._account = account;
    this._roles = roles;
  }

  public get account(): Beam.DirectoryEntry {
    if(!this.is_loaded) {
      throw Error('Model not loaded.');
    }
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    if(!this.is_loaded) {
      throw Error('Model not loaded.');
    }
    return this._roles;
  }

  public async load(): Promise<void> {
    this.is_loaded = true;
  }

  private is_loaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
