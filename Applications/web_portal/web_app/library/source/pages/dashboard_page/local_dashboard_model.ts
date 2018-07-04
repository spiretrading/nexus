import * as Beam from 'beam';
import * as Nexus from 'nexus';
import {DashboardModel} from '.';

/** Implements the DashboardModel using local memory. */
export class LocalDashboardModel extends DashboardModel {

  /** Constructs a LocalDashboardModel.
   * @param account - The account that's logged in.
   * @param roles - The account's roles.
   */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles) {
    super();
    this._account = account;
    this._roles = roles;
  }

  public get account(): Beam.DirectoryEntry {
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    return this._roles;
  }

  public async load(): Promise<void> {
    return;
  }

  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
