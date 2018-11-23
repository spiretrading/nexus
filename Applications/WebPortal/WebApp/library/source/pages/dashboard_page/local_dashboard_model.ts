import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { LocalAccountModel } from '../account_page';
import { DashboardModel } from '.';

/** Implements the DashboardModel using local memory. */
export class LocalDashboardModel extends DashboardModel {

  /** Constructs a LocalDashboardModel.
   * @param account - The account that's logged in.
   * @param roles - The account's roles.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._roles = roles;
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get account(): Beam.DirectoryEntry {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._roles;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): LocalAccountModel {
    if(account.equals(this._account)) {
      return new LocalAccountModel(this.account, this.roles);
    }
    return new LocalAccountModel(account, new Nexus.AccountRoles(0));
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public async logout(): Promise<void> {
    return;
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
